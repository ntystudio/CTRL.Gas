// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#include "CTRLGasAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"

#include "CTRLGas/CTRLAbilitySystemComponent.h"
#include "CTRLGas/CTRLGas.h"
#include "CTRLGas/CTRLGasAbilitySourceInterface.h"
#include "CTRLGas/CTRLGasEffectContext.h"
#include "CTRLGas/CTRLGasPhysicalMaterialWithTags.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CTRLGasAbility)

static auto const CVarAbilitySystemShowMakeOutgoingGameplayEffectSpecs = IConsoleManager::Get().FindConsoleVariable(TEXT("AbilitySystem.ShowClientMakeOutgoingSpecs"));

// ReSharper disable once CppMemberFunctionMayBeConst
bool UCTRLGasAbility::TryActivate(bool const bAllowRemoteActivation)
{
	if (!ensureMsgf(IsInstantiated(), TEXT("%hs called on the CDO. NonInstanced abilities are deprecated, thus we always expect this to be called on an instanced object."), __func__))
	{
		return false;
	}

	if (auto const ASC = GetASC())
	{
		return ASC->TryActivateAbility(GetCurrentAbilitySpecHandle(), bAllowRemoteActivation);
	}
	return false;
}

void UCTRLGasAbility::TryActivateAbilityOnSpawn(FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilitySpec const& Spec) const
{
	bool const bIsPredicting = (GetCurrentActivationInfo().ActivationMode == EGameplayAbilityActivationMode::Predicting);

	// Try to activate if activation policy is on spawn.
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && (ActivationPolicy == ECTRLAbilityActivationPolicy::OnSpawn))
	{
		UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
		AActor const* AvatarActor = ActorInfo->AvatarActor.Get();

		// If avatar actor is torn off or about to die, don't try to activate until we get the new one.
		if (ASC && AvatarActor && !AvatarActor->GetTearOff() && (AvatarActor->GetLifeSpan() <= 0.0f))
		{
			bool const bIsLocalExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly);
			bool const bIsServerExecution = (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly) || (NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated);

			bool const bClientShouldActivate = ActorInfo->IsLocallyControlled() && bIsLocalExecution;
			bool const bServerShouldActivate = ActorInfo->IsNetAuthority() && bIsServerExecution;

			if (bClientShouldActivate || bServerShouldActivate)
			{
				ASC->TryActivateAbility(Spec.Handle);
			}
		}
	}
}

void UCTRLGasAbility::ActivateAbility(
	FGameplayAbilitySpecHandle const Handle,
	FGameplayAbilityActorInfo const* ActorInfo,
	FGameplayAbilityActivationInfo const ActivationInfo,
	FGameplayEventData const* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!IsActive()) { return; } // Ability was ended before it could finish activation
	for (auto const EffectClass : OnActivateEffects)
	{
		if (!EffectClass || !IsValid(EffectClass)) { continue; }
		auto const Spec = Super::MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel(Handle, ActorInfo));
		auto EffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
		if (EffectHandle.IsValid())
		{
			OnActivateEffectHandles.Add(EffectHandle);
		}
	}
}

bool UCTRLGasAbility::CommitAbility(
	FGameplayAbilitySpecHandle const Handle,
	FGameplayAbilityActorInfo const* ActorInfo,
	FGameplayAbilityActivationInfo const ActivationInfo,
	FGameplayTagContainer* OptionalRelevantTags
)
{
	if (!Super::CommitAbility(Handle, ActorInfo, ActivationInfo, OptionalRelevantTags)) { return false; }
	for (auto const EffectClass : CommitEffects)
	{
		if (!EffectClass || !IsValid(EffectClass)) { continue; }
		auto const Spec = Super::MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel(Handle, ActorInfo));
		if (!Spec.IsValid()) continue;
		auto EffectHandle = ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, Spec);
		if (!EffectHandle.IsValid()) continue;
		OnCommitEffectHandles.Add(EffectHandle);
	}
	return true;
}

void UCTRLGasAbility::EndAbility(
	FGameplayAbilitySpecHandle const Handle,
	FGameplayAbilityActorInfo const* ActorInfo,
	FGameplayAbilityActivationInfo const ActivationInfo,
	bool const bReplicateEndAbility,
	bool const bWasCancelled
)
{
	bool const bEndAbilityValid = IsEndAbilityValid(Handle, ActorInfo);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	if (bEndAbilityValid)
	{
		K2_OnEndAbilityEvent(bWasCancelled);
		ClearEffects();
	}
}

UCTRLAbilitySystemComponent* UCTRLGasAbility::GetASC() const
{
	return Cast<UCTRLAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}

UCTRLGasAbility::UCTRLGasAbility(FObjectInitializer const& ObjectInitializer): Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; // Default to instanced per actor
}

void UCTRLGasAbility::ClearOnActivateEffects()
{
	auto const ASC = GetASC();
	if (!ASC) return;
	ASC->RemoveActiveEffectsForHandles(OnCommitEffectHandles);
}

void UCTRLGasAbility::ClearOnCommitEffects()
{
	auto const ASC = GetASC();
	if (!ASC) return;
	ASC->RemoveActiveEffectsForHandles(OnCommitEffectHandles);
}

FGameplayAbilityTargetingLocationInfo UCTRLGasAbility::MakeTargetingLocationInfoFromCamera()
{
	auto Info = MakeTargetLocationInfoFromOwnerActor();
	Info.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;
	if (auto const PC = GetActorInfo().PlayerController.Get())
	{
		Info.LiteralTransform.SetLocation(PC->PlayerCameraManager->GetCameraLocation());
		Info.LiteralTransform.SetRotation(PC->PlayerCameraManager->GetCameraRotation().Quaternion());
	}
	else
	{
		CTRL_GAS_LOG(Error, TEXT("Failed to get player controller from actor info in %s"), *GetNameSafe(this));
		Info.LiteralTransform.SetLocation(GetActorInfo().AvatarActor->GetActorLocation());
		Info.LiteralTransform.SetRotation(GetActorInfo().AvatarActor->GetActorRotation().Quaternion());
	}
	return Info;
}

template <typename T>
T* UCTRLGasAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (auto* PC = CurrentActorInfo->PlayerController.Get())
		{
			return Cast<T>(PC);
		}

		// Look for a player controller or pawn in the owner chain.
		auto* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (auto* C = Cast<T>(TestActor))
			{
				return C;
			}

			if (auto const* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController<T>();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

AController* UCTRLGasAbility::K2_GetControllerFromActorInfo(TSubclassOf<AController> const ControllerClass) const
{
	if (auto const Controller = GetControllerFromActorInfo())
	{
		if (Controller->IsA(ControllerClass))
		{
			return Controller;
		}
	}
	return nullptr;
}

ACharacter* UCTRLGasAbility::K2_GetCharacterFromActorInfo(TSubclassOf<AController> const CharacterClass) const
{
	if (auto const Character = GetCharacterFromActorInfo<ACharacter>())
	{
		if (Character->IsA(CharacterClass))
		{
			return Character;
		}
	}
	return nullptr;
}

template <typename T>
T* UCTRLGasAbility::GetCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<T>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
}

void UCTRLGasAbility::ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const
{
	Super::ApplyAbilityTagsToGameplayEffectSpec(Spec, AbilitySpec);

	if (auto const* HitResult = Spec.GetContext().GetHitResult())
	{
		if (auto const* PhysMatWithTags = Cast<UCTRLGasPhysicalMaterialWithTags const>(HitResult->PhysMaterial.Get()))
		{
			Spec.CapturedTargetTags.GetSpecTags().AppendTags(PhysMatWithTags->Tags);
		}
	}
}

void UCTRLGasAbility::OnGiveAbility(FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilitySpec const& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);
	K2_OnGiveAbility();
	TryActivateAbilityOnSpawn(ActorInfo, Spec);
}

void UCTRLGasAbility::OnRemoveAbility(FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilitySpec const& Spec)
{
	K2_OnRemoveAbility();
	Super::OnRemoveAbility(ActorInfo, Spec);
}

void UCTRLGasAbility::ClearEffects()
{
	ClearOnActivateEffects();
	ClearOnCommitEffects();
}

void UCTRLGasAbility::GetAbilitySource(
	FGameplayAbilitySpecHandle const Handle,
	FGameplayAbilityActorInfo const* ActorInfo,
	float& OutSourceLevel,
	ICTRLGasAbilitySourceInterface const*& OutAbilitySource,
	AActor*& OutEffectCauser
) const
{
	OutSourceLevel = 0.0f;
	OutAbilitySource = nullptr;
	OutEffectCauser = nullptr;

	OutEffectCauser = ActorInfo->AvatarActor.Get();

	// If we were added by something that's an ability info source, use it
	auto* SourceObject = GetSourceObject(Handle, ActorInfo);

	OutAbilitySource = Cast<ICTRLGasAbilitySourceInterface>(SourceObject);
}

FGameplayEffectContextHandle UCTRLGasAbility::MakeEffectContext(FGameplayAbilitySpecHandle const Handle, FGameplayAbilityActorInfo const* ActorInfo) const
{
	auto ContextHandle = Super::MakeEffectContext(Handle, ActorInfo);
	auto* EffectContext = FCTRLGasEffectContext::ExtractEffectContext(ContextHandle);
	check(EffectContext);

	check(ActorInfo);

	AActor* EffectCauser = nullptr;
	ICTRLGasAbilitySourceInterface const* AbilitySource = nullptr;
	float SourceLevel = 0.0f;
	GetAbilitySource(Handle, ActorInfo, /*out*/ SourceLevel, /*out*/ AbilitySource, /*out*/ EffectCauser);

	auto const* SourceObject = GetSourceObject(Handle, ActorInfo);

	auto* Instigator = ActorInfo ? ActorInfo->OwnerActor.Get() : nullptr;

	EffectContext->SetAbilitySource(AbilitySource, SourceLevel);
	EffectContext->AddInstigator(Instigator, EffectCauser);
	EffectContext->AddSourceObject(SourceObject);

	return ContextHandle;
}

FGameplayEffectSpecHandle UCTRLGasAbility::MakeOutgoingGameplayEffectSpec(
	FGameplayAbilitySpecHandle const Handle,
	FGameplayAbilityActorInfo const* ActorInfo,
	FGameplayAbilityActivationInfo const ActivationInfo,
	TSubclassOf<UGameplayEffect> const GameplayEffectClass,
	float Level
) const
{
	Level = Level == -1 ? GetAbilityLevel() : Level;
	if (!ensure(ActorInfo))
	{
		return FGameplayEffectSpecHandle{};
	}

	if (auto const ASC = Cast<UCTRLAbilitySystemComponent>(ActorInfo->AbilitySystemComponent.Get()))
	{
	#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (CVarAbilitySystemShowMakeOutgoingGameplayEffectSpecs->GetBool() && HasAuthority(&ActivationInfo) == false)
		{
			ABILITY_LOG(Warning, TEXT("%s, MakeOutgoingGameplayEffectSpec: %s"), *ASC->GetFullName(),  *GameplayEffectClass->GetName()); 
		}
	#endif

		FGameplayEffectSpecHandle NewHandle = ASC->MakeEffectSpec(GameplayEffectClass, Level, MakeEffectContext(Handle, ActorInfo));
		if (!NewHandle.IsValid()) return NewHandle;
		auto* AbilitySpec = ASC->FindAbilitySpecFromHandle(Handle);
		ApplyAbilityTagsToGameplayEffectSpec(*NewHandle.Data.Get(), AbilitySpec);

		// Copy over set by caller magnitudes
		if (AbilitySpec)
		{
			NewHandle.Data->SetByCallerTagMagnitudes = AbilitySpec->SetByCallerTagMagnitudes;
		}
		return NewHandle;
	}
	
	return Super::MakeOutgoingGameplayEffectSpec(Handle, ActorInfo, ActivationInfo, GameplayEffectClass, Level);
}

bool UCTRLGasAbility::CheckCost(FGameplayAbilitySpecHandle const Handle, FGameplayAbilityActorInfo const* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!CostGameplayEffectClass) { return true; }
	if (auto const ASC = Cast<UCTRLAbilitySystemComponent>(ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr))
	{
		auto const EffectSpec = ASC->MakeEffectSpec(CostGameplayEffectClass, GetAbilityLevel(), MakeEffectContext(Handle, ActorInfo));
		if (!EffectSpec.IsValid()) { return false; }
		if (!ASC->CanApplyAttributeModifiers(EffectSpec))
		{
			FGameplayTag const& CostTag = UAbilitySystemGlobals::Get().ActivateFailCostTag;
			if (OptionalRelevantTags && CostTag.IsValid())
			{
				OptionalRelevantTags->AddTag(CostTag);
			}
			return false;
		}
		return true;
	}

	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

void UCTRLGasAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (auto const ASC = Cast<UCTRLAbilitySystemComponent>(ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr))
	{
		auto const EffectSpec = ASC->MakeEffectSpec(CostGameplayEffectClass, GetAbilityLevel(), MakeEffectContext(Handle, ActorInfo));
		if (!EffectSpec.IsValid()) { return; }
		// ReSharper disable once CppExpressionWithoutSideEffects
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpec);
		return;
	}
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);
}

bool UCTRLGasAbility::CheckCooldown(FGameplayAbilitySpecHandle const Handle, FGameplayAbilityActorInfo const* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CheckCooldown(Handle, ActorInfo, OptionalRelevantTags);
}


void UCTRLGasAbility::ApplyCooldown(FGameplayAbilitySpecHandle const Handle, FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilityActivationInfo const ActivationInfo) const
{
	if (auto const ASC = Cast<UCTRLAbilitySystemComponent>(ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr))
	{
		auto const EffectSpec = ASC->MakeEffectSpec(CooldownGameplayEffectClass, GetAbilityLevel(), MakeEffectContext(Handle, ActorInfo));
		// ReSharper disable once CppExpressionWithoutSideEffects
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, EffectSpec);
		return;
	}
	Super::ApplyCooldown(Handle, ActorInfo, ActivationInfo);
}