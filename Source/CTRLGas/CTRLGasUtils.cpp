﻿// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#include "CTRLGasUtils.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "GameplayPrediction.h"

#include "CTRLCore/CTRLActorUtils.h"

#include "CTRLGas/CTRLGas.h"

#include "GameFramework/Actor.h"

#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CTRLGasUtils)

void UCTRLGasUtils::AssignAbilitySetByCallerMagnitude(UGameplayAbility* Ability, FGameplayTag const MagnitudeTag, double const MagnitudeValue)
{
	if (!Ability) return;
	if (!MagnitudeTag.IsValid()) return;
	auto const Spec = Ability->GetCurrentAbilitySpec();
	if (!Spec) return;
	Spec->SetByCallerTagMagnitudes.Add(MagnitudeTag, MagnitudeValue);
}

FPredictionKey UCTRLGasUtils::GetOriginalPredictionKey(FGameplayAbilitySpec const& Spec)
{
	auto const* Instance = Spec.GetPrimaryInstance();
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	// ReSharper disable once CppDeprecatedEntity
	return Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

bool UCTRLGasUtils::IsValidAbilitySpec(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle)
{
	if (!ASC) return false;
	if (!Handle.IsValid()) return false;
	auto const* AbilitySpec = ASC->FindAbilitySpecFromHandle(Handle);
	return !!(AbilitySpec && IsValid(AbilitySpec->Ability));
}

FGameplayEffectSpec UCTRLGasUtils::K2_GetEffectSpec(FGameplayEffectSpecHandle const& Handle, bool& bSuccess)
{
	bSuccess = false;
	if (auto const EffectSpec = GetEffectSpec(Handle))
	{
		if (auto const Spec = EffectSpec.Get(nullptr))
		{
			bSuccess = true;
			return *Spec;
		}
	}
	return {};
}

TOptional<TSharedPtr<FGameplayEffectSpec>> UCTRLGasUtils::GetEffectSpec(FGameplayEffectSpecHandle const& Handle)
{
	if (!Handle.IsValid()) { return {}; }
	return Handle.Data;
}

UGameplayAbility* UCTRLGasUtils::K2_GetAbilityCDO(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle, TSubclassOf<UGameplayAbility> const Class, bool& bSuccess)
{
	bSuccess = false;
	if (!IsValid(ASC)) { return nullptr; }
	if (auto const* AbilitySpec = ASC->FindAbilitySpecFromHandle(Handle))
	{
		auto const Ability = AbilitySpec->Ability;
		if (Ability && Ability->IsA(Class))
		{
			bSuccess = true;
			return Ability.Get();
		}
	}
	return nullptr;
}

UGameplayAbility* UCTRLGasUtils::K2_GetAbilityPrimaryInstance(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle, TSubclassOf<UGameplayAbility> const Class, bool& bSuccess)
{
	bSuccess = false;
	if (!IsValid(ASC)) { return nullptr; }
	if (auto const* AbilitySpec = ASC->FindAbilitySpecFromHandle(Handle))
	{
		auto const Ability = AbilitySpec->GetPrimaryInstance();
		if (Ability && Ability->IsA(Class))
		{
			bSuccess = true;
			return Ability;
		}
	}
	return nullptr;
}

FGameplayAbilitySpecHandle UCTRLGasUtils::K2_GetAbilitySpecHandle(UGameplayAbility const* Ability, bool& bSuccess)
{
	bSuccess = false;
	if (!Ability) { return {}; }
	auto const Handle = GetAbilitySpecHandle(Ability);
	if (Handle.IsValid())
	{
		bSuccess = true;
	}
	return Handle;
}

FGameplayAbilitySpecHandle UCTRLGasUtils::GetAbilitySpecHandle(UGameplayAbility const* Ability)
{
	if (!Ability) { return {}; }
	if (auto const* Spec = Ability->GetCurrentAbilitySpec())
	{
		return Spec->Handle;
	}
	return {};
}

FActiveGameplayEffectHandle UCTRLGasUtils::AddGameplayTagsEffect(UAbilitySystemComponent* ASC, FGameplayTagContainer const& Tags, UObject* SourceObject)
{
	ensure(ASC);
	auto EffectContext = ASC->MakeEffectContext();
	if (SourceObject)
	{
		EffectContext.AddSourceObject(SourceObject);
	}
	auto const* TagsEffect = MakeEffect(
		ASC,
		TEXT("AddGameplayTagsEffect"),
		[&Tags](UGameplayEffect* Effect)
		{
			FInheritedTagContainer TagChanges;
			for (auto const& Tag : Tags)
			{
				TagChanges.AddTag(Tag);
			}
			auto& GrantTagsComponent = Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
			GrantTagsComponent.SetAndApplyTargetTagChanges(TagChanges);
			return Effect;
		}
	);
	CTRL_GAS_LOG(Verbose, TEXT("%s: Add: %s"), *TagsEffect->GetName(), *Tags.ToStringSimple());
	return ASC->ApplyGameplayEffectToSelf(TagsEffect, 1.0f, EffectContext);
}

FActiveGameplayEffectHandle UCTRLGasUtils::AddInheritedGameplayTagsEffect(UAbilitySystemComponent* ASC, FInheritedTagContainer const& TagChanges, UObject* SourceObject)
{
	ensure(ASC);
	auto EffectContext = ASC->MakeEffectContext();
	if (SourceObject)
	{
		EffectContext.AddSourceObject(SourceObject);
	}
	auto const* TagsEffect = MakeEffect(
		ASC,
		TEXT("AddInheritedGameplayTagsEffect"),
		[&TagChanges](UGameplayEffect* Effect)
		{
			auto& GrantTagsComponent = Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
			GrantTagsComponent.SetAndApplyTargetTagChanges(TagChanges);
			return Effect;
		}
	);
	CTRL_GAS_LOG(Verbose, TEXT("%s: Add: %s Remove: %s"), *TagsEffect->GetName(), *TagChanges.Added.ToStringSimple(), *TagChanges.Removed.ToStringSimple());
	return ASC->ApplyGameplayEffectToSelf(TagsEffect, 1.0f, EffectContext);
}

FActiveGameplayEffectHandle UCTRLGasUtils::K2_AddInheritedGameplayTagsEffectDuration(UAbilitySystemComponent* ASC, FInheritedTagContainer const& TagChanges, float Duration, UObject* SourceObject)
{
	return AddInheritedGameplayTagsEffectDuration(ASC, TagChanges, FGameplayEffectModifierMagnitude(Duration), SourceObject);
}

UGameplayEffect* UCTRLGasUtils::MakeEffect(UAbilitySystemComponent* ASC, FName const& BaseName, FCTRLGasEffectFactoryFn Factory)
{
	FGameplayEffectModifierMagnitude Duration;
	auto const Name = MakeUniqueObjectName(ASC, UGameplayEffect::StaticClass(), BaseName);
	auto Effect = NewObject<UGameplayEffect>(ASC, Name);
	Effect->DurationPolicy = EGameplayEffectDurationType::Infinite;
	Effect->StackingType = EGameplayEffectStackingType::AggregateByTarget;
	Effect->StackLimitCount = 1;
	if (Factory)
	{
		Effect = Factory(Effect);
	}
	return Effect;
}

FActiveGameplayEffectHandle UCTRLGasUtils::AddInheritedGameplayTagsEffectDuration(
	UAbilitySystemComponent* ASC,
	FInheritedTagContainer const& TagChanges,
	FGameplayEffectModifierMagnitude Duration,
	UObject const* SourceObject
)
{
	ensure(ASC);
	auto EffectContext = ASC->MakeEffectContext();
	if (SourceObject)
	{
		EffectContext.AddSourceObject(SourceObject);
	}

	auto const* TagsEffect = MakeEffect(
		ASC,
		TEXT("AddInheritedGameplayTagsEffectDuration"),
		[&TagChanges, &Duration](UGameplayEffect* Effect)
		{
			Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
			Effect->DurationMagnitude = Duration;
			auto& GrantTagsComponent = Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
			GrantTagsComponent.SetAndApplyTargetTagChanges(TagChanges);
			return Effect;
		}
	);
	CTRL_GAS_LOG(Verbose, TEXT("%s: Add: %s Remove: %s"), *TagsEffect->GetName(), *TagChanges.Added.ToStringSimple(), *TagChanges.Removed.ToStringSimple());
	return ASC->ApplyGameplayEffectToSelf(TagsEffect, 1.0f, EffectContext);
}

void UCTRLGasUtils::GetActorGameplayTags(AActor const* Actor, FGameplayTagContainer& OutGameplayTags)
{
	OutGameplayTags.AppendTags(GetGameplayTags(Actor));
}

// UObject* UCTRLGasUtils::GetGameplayTagSource(UObject const* Object, bool const bWalkOuter)
// {
// 	if (!IsValid(Object)) { return nullptr; }
// 	auto CurrentTargetObject = Object;
// 	// check object outer chain for IGameplayTagAssetInterface
// 	while (CurrentTargetObject)
// 	{
// 		if (CurrentTargetObject->Implements<UGameplayTagAssetInterface>())
// 		{
// 			return const_cast<UObject*>(CurrentTargetObject);
// 		}
// 		if (!bWalkOuter) break; // only check the passed object
// 		CurrentTargetObject = CurrentTargetObject->GetOuter();
// 	}
//
// 	if (auto const Actor = UCTRLActorUtils::GetActor<AActor>(Object))
// 	{
// 		if (Actor->Implements<UGameplayTagAssetInterface>())
// 		{
// 			return const_cast<AActor*>(Actor);
// 		}
// 		if (auto const ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
// 		{
// 			return const_cast<UAbilitySystemComponent*>(ASC);
// 		}
// 	}
// 	return nullptr;
// }

FGameplayTagContainer UCTRLGasUtils::GetGameplayTags(UObject const* Object, bool const bWalkOuter)
{
	FGameplayTagContainer Tags;
	if (!IsValid(Object)) { return MoveTemp(Tags); }
	
	auto CurrentTargetObject = Object;
	// check object outer chain for IGameplayTagAssetInterface
	while (CurrentTargetObject)
	{
		if (auto const AsGameplayTagAssetInterface = Cast<IGameplayTagAssetInterface>(CurrentTargetObject))
		{
			AsGameplayTagAssetInterface->GetOwnedGameplayTags(Tags);
			return MoveTemp(Tags);
		}
		if (!bWalkOuter) break; // only check the passed object
		CurrentTargetObject = CurrentTargetObject->GetOuter();
	}

	if (auto const Actor = UCTRLActorUtils::GetActor(Object))
	{
		if (auto const AsGameplayTagAssetInterface = Cast<IGameplayTagAssetInterface>(Actor))
		{
			AsGameplayTagAssetInterface->GetOwnedGameplayTags(Tags);
			return MoveTemp(Tags);
		}
		if (auto const ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
		{
			ASC->GetOwnedGameplayTags(Tags);
			return MoveTemp(Tags);
		}
	}
	return MoveTemp(Tags);
}

TArray<FGameplayAttribute> UCTRLGasUtils::K2_ListAttributes(TSubclassOf<UAttributeSet> const AttributeSet)
{
	auto Attributes = TArray<FGameplayAttribute>();
	UAttributeSet::GetAttributesFromSetClass(AttributeSet, Attributes);
	return MoveTemp(Attributes);
}

template <typename T>
TArray<FGameplayAttribute> UCTRLGasUtils::ListAttributes()
{
	return K2_ListAttributes(T::StaticClass());
}

TArray<UAttributeSet*> UCTRLGasUtils::FindAttributeSets(AActor const* Target)
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
