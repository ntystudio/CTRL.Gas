// SPDX-FileCopyrightText: 2025 NTY.studio

#include "UseGAS/UseGASAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"

#include "UseGAS/UseGAS.h"
#include "UseGAS/UseGASUtils.h"
#include "UseGAS/Abilities/UseGASAbility.h"

int32 UUseGASAbilitySystemComponent::GetItemLevel(float const InLevel) const
{
	return InLevel == -1.0f ? GetLevel() : FMath::Max(1, FMath::Floor(InLevel));
}

int32 UUseGASAbilitySystemComponent::GetItemLevel(int32 const InLevel) const
{
	return InLevel == -1 ? GetLevel() : FMath::Max(1, InLevel);
}

UUseGASAbilitySystemComponent* UUseGASAbilitySystemComponent::Get(UObject const* SourceObject, bool bWarnIfNotFound)
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
			USEGAS_LOG(Warning, TEXT("Get: Actor %s has an ASC but it is not a %s."), *GetNameSafe(Actor), *ThisClass::StaticClass()->GetName());
		}
		else
		{
			USEGAS_LOG(Warning, TEXT("Get: Actor %s does not have a %s."), *GetNameSafe(Actor), *ThisClass::StaticClass()->GetName());
		}
	}
	return OurASC;
}

UUseGASAbilitySystemComponent* UUseGASAbilitySystemComponent::GetChecked(UObject const* SourceObject)
{
	auto const ASC = Get(SourceObject);
	check(IsValid(ASC));
	return ASC;
}

UUseGASAbilitySystemComponent* UUseGASAbilitySystemComponent::GetEnsured(UObject const* SourceObject)
{
	auto const ASC = Get(SourceObject);
	ensure(IsValid(ASC));
	return ASC;
}

TArray<UAttributeSet*> UUseGASAbilitySystemComponent::FindAttributes(AActor const* Target)
{
	TArray<UObject*> ChildObjects;
	GetObjectsWithOuter(Target, ChildObjects, false, RF_NoFlags, EInternalObjectFlags::Garbage);
	TArray<UAttributeSet*> Attributes;
	for (auto* Obj : ChildObjects)
	{
		if (auto* AttributeSet = Cast<UAttributeSet>(Obj))
		{
			Attributes.AddUnique(AttributeSet);
		}
	}
	return MoveTemp(Attributes);
}

TArray<FGameplayAttribute> UUseGASAbilitySystemComponent::ListAttributes(TSubclassOf<UAttributeSet> AttributeSet)
{
	auto Attributes = TArray<FGameplayAttribute>();
	UAttributeSet::GetAttributesFromSetClass(AttributeSet, Attributes);
	return MoveTemp(Attributes);
}

void UUseGASAbilitySystemComponent::RemoveActiveEffectsForHandles(TArray<FActiveGameplayEffectHandle>& Handles)
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

void UUseGASAbilitySystemComponent::OnTagUpdated(FGameplayTag const& Tag, bool const TagExists)
{
	Super::OnTagUpdated(Tag, TagExists);
	OnTagUpdatedDelegate.Broadcast(Tag, TagExists);
}

void UUseGASAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool const bReplicateCancelAbility)
{
	ABILITYLIST_SCOPE_LOCK();
	for (auto const& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		auto* AbilityCDO = Cast<UUseGASAbility>(AbilitySpec.Ability);
		if (!AbilityCDO)
		{
			USEGAS_LOG(Error, TEXT("CancelAbilitiesByFunc: Non-UseGASAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
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
				auto const Ability = CastChecked<UUseGASAbility>(AbilityInstance);

				if (ShouldCancelFunc(Ability, AbilitySpec.Handle))
				{
					if (Ability->CanBeCanceled())
					{
						Ability->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), Ability->GetCurrentActivationInfo(), bReplicateCancelAbility);
					}
					else
					{
						USEGAS_LOG(Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *Ability->GetName());
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

void UUseGASAbilitySystemComponent::CancelInputActivatedAbilities(bool const bReplicateCancelAbility)
{
	auto ShouldCancelFunc = [this](UUseGASAbility const* Ability, FGameplayAbilitySpecHandle Handle)
	{
		auto const ActivationPolicy = Ability->GetActivationPolicy();
		return ActivationPolicy == ELyraAbilityActivationPolicy::OnInputTriggered || ActivationPolicy == ELyraAbilityActivationPolicy::WhileInputActive;
	};

	CancelAbilitiesByFunc(ShouldCancelFunc, bReplicateCancelAbility);
}

void UUseGASAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputPress ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, UUseGASUtils::GetOriginalPredictionKey(Spec));
	}
}

void UUseGASAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	// We don't support UGameplayAbility::bReplicateInputDirectly.
	// Use replicated events instead so that the WaitInputRelease ability task works.
	if (Spec.IsActive())
	{
		// Invoke the InputReleased event. This is not replicated here. If someone is listening, they may replicate the InputReleased event to the server.
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, UUseGASUtils::GetOriginalPredictionKey(Spec));
	}
}

void UUseGASAbilitySystemComponent::OnAbilityInputTagPressed(FGameplayTag const& InputTag)
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

void UUseGASAbilitySystemComponent::OnAbilityInputTagReleased(FGameplayTag const& InputTag)
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

void UUseGASAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
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
				auto const* AbilityCDO = Cast<UUseGASAbility>(AbilitySpec->Ability);
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
					auto const* AbilityCDO = Cast<UUseGASAbility>(AbilitySpec->Ability);

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

void UUseGASAbilitySystemComponent::ClearAbilityInput()
{
	InputPressedSpecHandles.Reset();
	InputReleasedSpecHandles.Reset();
	InputHeldSpecHandles.Reset();
}

FActiveGameplayEffectHandle UUseGASAbilitySystemComponent::ApplySpecToSelf(
	UObject const* InSourceObject,
	TSubclassOf<UGameplayEffect> const EffectClass,
	TOptional<FUseGASEffectSpecFactoryFn> const SpecFactory
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
		Spec = SpecFactory.GetValue()(Spec);
	}
	if (!Spec.IsValid()) return {};
	return ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

//~ Adapted from FActiveGameplayEffectsContainer::CanApplyAttributeModifiers
bool UUseGASAbilitySystemComponent::CanApplyAttributeModifiers(FGameplayEffectSpecHandle const& SpecHandle) const
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
