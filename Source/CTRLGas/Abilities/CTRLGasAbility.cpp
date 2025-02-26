﻿// SPDX-FileCopyrightText: 2025 NTY.studio

#include "CTRLGasAbility.h"

#include "AbilitySystemComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

#include "CTRLGas/CTRLGasAbilitySourceInterface.h"
#include "CTRLGas/CTRLGas.h"
#include "CTRLGas/CTRLAbilitySystemComponent.h"
#include "CTRLGas/CTRLGasEffectContext.h"
#include "CTRLGas/CTRLGasPhysicalMaterialWithTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CTRLGasAbility)

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
	if (ActorInfo && !Spec.IsActive() && !bIsPredicting && (ActivationPolicy == ELyraAbilityActivationPolicy::OnSpawn))
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
		auto const Spec = MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel(Handle, ActorInfo));
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
		auto const Spec = MakeOutgoingGameplayEffectSpec(EffectClass, GetAbilityLevel(Handle, ActorInfo));
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

AController* UCTRLGasAbility::GetControllerFromActorInfo() const
{
	if (CurrentActorInfo)
	{
		if (auto* PC = CurrentActorInfo->PlayerController.Get())
		{
			return PC;
		}

		// Look for a player controller or pawn in the owner chain.
		auto* TestActor = CurrentActorInfo->OwnerActor.Get();
		while (TestActor)
		{
			if (auto* C = Cast<AController>(TestActor))
			{
				return C;
			}

			if (auto const* Pawn = Cast<APawn>(TestActor))
			{
				return Pawn->GetController();
			}

			TestActor = TestActor->GetOwner();
		}
	}

	return nullptr;
}

ACharacter* UCTRLGasAbility::GetCharacterFromActorInfo() const
{
	return (CurrentActorInfo ? Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get()) : nullptr);
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
