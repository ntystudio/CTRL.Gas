// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#include "CTRLGas/CTRLAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"

#include "CTRLGas/CTRLGas.h"
#include "CTRLGas/CTRLGasUtils.h"
#include "CTRLGas/Abilities/CTRLGasAbility.h"

int32 UCTRLAbilitySystemComponent::GetLevelOrDefault(float const InLevel) const
{
	return InLevel == -1.0f ? GetLevel() : FMath::Max(1, FMath::Floor(InLevel));
}

int32 UCTRLAbilitySystemComponent::GetLevelOrDefault(int32 const InLevel) const
{
	return InLevel == -1 ? GetLevel() : FMath::Max(1, InLevel);
}

int32 UCTRLAbilitySystemComponent::GetLevelOrDefault(TOptional<int32> InLevel) const
{
	if (InLevel.IsSet())
	{
		return GetLevelOrDefault(InLevel.GetValue());
	}
	return GetLevel();
}

int32 UCTRLAbilitySystemComponent::GetLevelOrDefault(TOptional<float> InLevel) const
{
	if (InLevel.IsSet())
	{
		return GetLevelOrDefault(InLevel.GetValue());
	}
	return GetLevel();
}

UCTRLAbilitySystemComponent* UCTRLAbilitySystemComponent::Get(UObject const* SourceObject, bool const bWarnIfNotFound)
{
	auto* Actor = Cast<AActor>(SourceObject);
	if (!Actor)
	{
		if (auto const Component = Cast<UActorComponent>(SourceObject))
		{
			Actor = Component->GetOwner();
		}
		else
		{
			Actor = SourceObject->GetTypedOuter<AActor>();
		}
	}
	auto const ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	auto const OurASC = Cast<ThisClass>(ASC);
	if (bWarnIfNotFound && !OurASC)
	{
		if (ASC)
		{
			CTRL_GAS_LOG(Warning, TEXT("Get: Actor %s has an ASC but it is not a %s."), *GetNameSafe(Actor), *ThisClass::StaticClass()->GetName());
		}
		else
		{
			CTRL_GAS_LOG(Warning, TEXT("Get: Actor %s does not have a %s."), *GetNameSafe(Actor), *ThisClass::StaticClass()->GetName());
		}
	}
	return OurASC;
}

UCTRLAbilitySystemComponent* UCTRLAbilitySystemComponent::GetChecked(UObject const* SourceObject)
{
	auto const ASC = Get(SourceObject);
	check(IsValid(ASC));
	return ASC;
}

UCTRLAbilitySystemComponent* UCTRLAbilitySystemComponent::GetEnsured(UObject const* SourceObject)
{
	auto const ASC = Get(SourceObject);
	ensure(IsValid(ASC));
	return ASC;
}

void UCTRLAbilitySystemComponent::RemoveActiveEffectsForHandles(TArray<FActiveGameplayEffectHandle>& Handles)
{
	if (!Handles.Num()) return;
	auto CurrentHandles = Handles;
	Handles.Reset();
	for (auto const EffectHandle : CurrentHandles)
	{
		if (EffectHandle.IsValid())
		{
			RemoveActiveGameplayEffect(EffectHandle);
		}
	}
}

void UCTRLAbilitySystemComponent::OnTagUpdated(FGameplayTag const& Tag, bool const TagExists)
{
	Super::OnTagUpdated(Tag, TagExists);
	OnTagUpdatedDelegate.Broadcast(Tag, TagExists);
}

void UCTRLAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool const bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (auto const& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		auto* AbilityCDO = Cast<UCTRLGasAbility>(AbilitySpec.Ability);
		if (!AbilityCDO)
		{
			CTRL_GAS_LOG(Error, TEXT("CancelAbilitiesByFunc: Non-CTRLGasAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
			continue;
		}

		// ReSharper disable once CppDeprecatedEntity
		PRAGMA_DISABLE_DEPRECATION_WARNINGS
		if (AbilityCDO->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced)
		{
			// Cancel all the spawned instances, not the CDO.
			auto Instances = AbilitySpec.GetAbilityInstances();
			for (auto* AbilityInstance : Instances)
			{
				auto const Ability = CastChecked<UCTRLGasAbility>(AbilityInstance);

				if (ShouldCancelFunc(Ability, AbilitySpec.Handle))
				{
					if (Ability->CanBeCanceled())
					{
						Ability->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), Ability->GetCurrentActivationInfo(), bReplicateCancelAbility);
					}
					else
					{
						CTRL_GAS_LOG(Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *Ability->GetName());
					}
				}
			}
		}
		else
		{
			// Cancel the non-instanced ability CDO.
			if (ShouldCancelFunc(AbilityCDO, AbilitySpec.Handle))
			{
				// Non-instanced abilities can always be canceled.
				check(AbilityCDO->CanBeCanceled());
				AbilityCDO->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FGameplayAbilityActivationInfo(), bReplicateCancelAbility);
			}
		}
	}
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

void UCTRLAbilitySystemComponent::CancelInputActivatedAbilities(bool const bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](UCTRLGasAbility const* Ability, FGameplayAbilitySpecHandle Handle)
	{
		auto const ActivationPolicy = Ability->GetActivationPolicy();
		return ActivationPolicy == ELyraAbilityActivationPolicy::OnInputTriggered || ActivationPolicy == ELyraAbilityActivationPolicy::WhileInputActive;
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UCTRLAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, UCTRLGasUtils::GetOriginalPredictionKey(Spec));
	}
}

void UCTRLAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, UCTRLGasUtils::GetOriginalPredictionKey(Spec));
	}
}

void UCTRLAbilitySystemComponent::OnAbilityInputTagPressed(FGameplayTag const& InputTag)
{
	if (!InputTag.IsValid()) return;
	for (auto const& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputPressedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.AddUnique(AbilitySpec.Handle);
		}
	}
}

void UCTRLAbilitySystemComponent::OnAbilityInputTagReleased(FGameplayTag const& InputTag)
{
	if (!InputTag.IsValid()) return;
	for (auto const& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			InputReleasedSpecHandles.AddUnique(AbilitySpec.Handle);
			InputHeldSpecHandles.Remove(AbilitySpec.Handle);
		}
	}
}

void UCTRLAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	// {
	// 	ClearAbilityInput();
	// 	return;
	// }

	static TArray<FGameplayAbilitySpecHandle> AbilitiesToActivate;
	AbilitiesToActivate.Reset();

	//@TODO: See if we can use FScopedServerAbilityRPCBatcher ScopedRPCBatcher in some of these loops

	// Process all abilities that activate when the input is held.
	for (auto const& SpecHandle : InputHeldSpecHandles)
	{
		if (auto const* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability && !AbilitySpec->IsActive())
			{
				auto const* AbilityCDO = Cast<UCTRLGasAbility>(AbilitySpec->Ability);
				if (AbilityCDO && AbilityCDO->GetActivationPolicy() == ELyraAbilityActivationPolicy::WhileInputActive)
				{
					AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
				}
			}
		}
	}

	// Process all abilities that had their input pressed this frame.
	for (auto const& SpecHandle : InputPressedSpecHandles)
	{
		if (auto* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = true;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputPressed(*AbilitySpec);
				}
				else
				{
					auto const* AbilityCDO = Cast<UCTRLGasAbility>(AbilitySpec->Ability);

					if (AbilityCDO && AbilityCDO->GetActivationPolicy() == ELyraAbilityActivationPolicy::OnInputTriggered)
					{
						AbilitiesToActivate.AddUnique(AbilitySpec->Handle);
					}
				}
			}
		}
	}

	// Try to activate all the abilities that are from presses and holds.
	// We do it all at once so that held inputs don't activate the ability
	// and then also send an input event to the ability because of the press.
	for (auto const& AbilitySpecHandle : AbilitiesToActivate)
	{
		TryActivateAbility(AbilitySpecHandle);
	}

	// Process all abilities that had their input released this frame.
	for (auto const& SpecHandle : InputReleasedSpecHandles)
	{
		if (auto* AbilitySpec = FindAbilitySpecFromHandle(SpecHandle))
		{
			if (AbilitySpec->Ability)
			{
				AbilitySpec->InputPressed = false;

				if (AbilitySpec->IsActive())
				{
					// Ability is active so pass along the input event.
					AbilitySpecInputReleased(*AbilitySpec);
				}
			}
		}
	}

	// Clear the cached ability handles.
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
}

void UCTRLAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles
		.Reset();
}

FGameplayEffectSpecHandle UCTRLAbilitySystemComponent::MakeSpec(
	TSubclassOf<UGameplayEffect> const& GameplayEffectClass,
	TOptional<float> const& Level,
	TOptional<FGameplayEffectContextHandle> const& Context
) const
{
	return MakeEffectSpec(GameplayEffectClass, Level, Context);
}

FGameplayEffectSpecHandle UCTRLAbilitySystemComponent::MakeEffectSpec(
	TSubclassOf<UGameplayEffect> const& GameplayEffectClass,
	TOptional<float> Level,
	TOptional<FGameplayEffectContextHandle> Context
) const
{
	return MakeOutgoingSpec(
		GameplayEffectClass,
		GetLevelOrDefault(Level),
		Context.IsSet() ? Context.GetValue() : MakeEffectContext()
	);
}

FGameplayAbilitySpec UCTRLAbilitySystemComponent::MakeAbilitySpec(TSubclassOf<UGameplayAbility> const& GameplayAbilityClass, TOptional<float> const& Level, UObject* Source) const
{
	return FGameplayAbilitySpec(GameplayAbilityClass, GetLevelOrDefault(Level), INDEX_NONE, Source);
}

FActiveGameplayEffectHandle UCTRLAbilitySystemComponent::ApplySpecToSelf(
	UObject const* InSourceObject,
	TSubclassOf<UGameplayEffect> const EffectClass,
	FCTRLGasEffectSpecFactoryFn const SpecFactory
)
{
	if (!IsValid(InSourceObject)) return {};
	if (!IsValid(EffectClass)) return {};

	auto EffectContext = MakeEffectContext();
	EffectContext.AddSourceObject(InSourceObject);
	auto Spec = MakeOutgoingSpec(EffectClass, GetLevel(), EffectContext);
	if (!Spec.IsValid()) return {};
	if (SpecFactory)
	{
		Spec = SpecFactory(Spec);
	}
	if (!Spec.IsValid()) return {};
	return ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

//~ Adapted from FActiveGameplayEffectsContainer::CanApplyAttributeModifiers to work on a spec handle directly
bool UCTRLAbilitySystemComponent::CanApplyAttributeModifiers(FGameplayEffectSpecHandle const& SpecHandle) const
{
	if (!SpecHandle.IsValid()) return false;
	auto Spec = *SpecHandle.Data.Get();
	Spec.CalculateModifierMagnitudes();
	for (int32 ModIdx = 0; ModIdx < Spec.Modifiers.Num(); ++ModIdx)
	{
		FGameplayModifierInfo const& ModDef = Spec.Def->Modifiers[ModIdx];
		FModifierSpec const& ModSpec = Spec.Modifiers[ModIdx];

		// It only makes sense to check additive operators
		if (ModDef.ModifierOp == EGameplayModOp::Additive)
		{
			if (!ModDef.Attribute.IsValid()) continue;
			UAttributeSet const* Set = GetAttributeSubobject(ModDef.Attribute.GetAttributeSetClass());
			float const CurrentValue = ModDef.Attribute.GetNumericValueChecked(Set);
			float const CostValue = ModSpec.GetEvaluatedMagnitude();

			if (CurrentValue + CostValue < 0.f)
			{
				return false;
			}
		}
	}
	return true;
}

bool UCTRLAbilitySystemComponent::CanApplyAttributeModifiers(TSubclassOf<UGameplayEffect> const& EffectClass, FCTRLGasEffectSpecFactoryFn const& SpecFactory) const
{
	auto SpecHandle = MakeEffectSpec(EffectClass);
	if (!SpecHandle.IsValid()) return false;
	if (SpecFactory)
	{
		SpecHandle = SpecFactory(SpecHandle);
	}

	return CanApplyAttributeModifiers(SpecHandle);
}
