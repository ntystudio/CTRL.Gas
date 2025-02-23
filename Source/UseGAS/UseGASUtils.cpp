// SPDX-FileCopyrightText: 2025 NTY.studio

#include "UseGAS/UseGASUtils.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayPrediction.h"

#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

#include "UseGAS/UseGAS.h"

void UUseGASUtils::AssignAbilitySetByCallerMagnitude(UGameplayAbility* Ability, FGameplayTag const MagnitudeTag, double const MagnitudeValue)
{
	if (!Ability) return;
	if (!MagnitudeTag.IsValid()) return;
	auto const Spec = Ability->GetCurrentAbilitySpec();
	if (!Spec) return;
	Spec->SetByCallerTagMagnitudes.Add(MagnitudeTag, MagnitudeValue);
}

FPredictionKey UUseGASUtils::GetOriginalPredictionKey(FGameplayAbilitySpec const& Spec)
{
	auto const* Instance = Spec.GetPrimaryInstance();
	PRAGMA_DISABLE_DEPRECATION_WARNINGS
	// ReSharper disable once CppDeprecatedEntity
	return Instance ? Instance->GetCurrentActivationInfo().GetActivationPredictionKey() : Spec.ActivationInfo.GetActivationPredictionKey();
	PRAGMA_ENABLE_DEPRECATION_WARNINGS
}

bool UUseGASUtils::IsValidAbilitySpec(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle)
{
	if (!ASC) return false;
	if (!Handle.IsValid()) return false;
	auto const* AbilitySpec = ASC->FindAbilitySpecFromHandle(Handle);
	return !!(AbilitySpec && IsValid(AbilitySpec->Ability));
}

FGameplayEffectSpec UUseGASUtils::K2_GetEffectSpec(FGameplayEffectSpecHandle const& Handle, bool& bSuccess)
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

TOptional<TSharedPtr<FGameplayEffectSpec>> UUseGASUtils::GetEffectSpec(FGameplayEffectSpecHandle const& Handle)
{
	if (!Handle.IsValid()) { return {}; }
	return Handle.Data;
}

UGameplayAbility* UUseGASUtils::K2_GetAbilityCDO(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle, TSubclassOf<UGameplayAbility> const Class, bool& bSuccess)
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

UGameplayAbility* UUseGASUtils::K2_GetAbilityPrimaryInstance(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle, TSubclassOf<UGameplayAbility> const Class, bool& bSuccess)
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

FGameplayAbilitySpecHandle UUseGASUtils::K2_GetAbilitySpecHandle(UGameplayAbility const* Ability, bool& bSuccess)
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

FGameplayAbilitySpecHandle UUseGASUtils::GetAbilitySpecHandle(UGameplayAbility const* Ability)
{
	if (!Ability) { return {}; }
	if (auto const* Spec = Ability->GetCurrentAbilitySpec())
	{
		return Spec->Handle;
	}
	return {};
}

FActiveGameplayEffectHandle UUseGASUtils::AddGameplayTagsEffect(UAbilitySystemComponent* ASC, FGameplayTagContainer const& Tags, UObject* SourceObject)
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
	USEGAS_LOG(Verbose, TEXT("%s: Add: %s"), *TagsEffect->GetName(), *Tags.ToStringSimple());
	return ASC->ApplyGameplayEffectToSelf(TagsEffect, 1.0f, EffectContext);
}

FActiveGameplayEffectHandle UUseGASUtils::AddInheritedGameplayTagsEffect(UAbilitySystemComponent* ASC, FInheritedTagContainer const& TagChanges, UObject* SourceObject)
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
	USEGAS_LOG(Verbose, TEXT("%s: Add: %s Remove: %s"), *TagsEffect->GetName(), *TagChanges.Added.ToStringSimple(), *TagChanges.Removed.ToStringSimple());
	return ASC->ApplyGameplayEffectToSelf(TagsEffect, 1.0f, EffectContext);
}

FActiveGameplayEffectHandle UUseGASUtils::K2_AddInheritedGameplayTagsEffectDuration(UAbilitySystemComponent* ASC, FInheritedTagContainer const& TagChanges, float Duration, UObject* SourceObject)
{
	return AddInheritedGameplayTagsEffectDuration(ASC, TagChanges, FGameplayEffectModifierMagnitude(Duration), SourceObject);
}

UGameplayEffect* UUseGASUtils::MakeEffect(UAbilitySystemComponent* ASC, FName const& BaseName, FUseGASEffectFactoryFn Factory)
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

FActiveGameplayEffectHandle UUseGASUtils::AddInheritedGameplayTagsEffectDuration(
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
	USEGAS_LOG(Verbose, TEXT("%s: Add: %s Remove: %s"), *TagsEffect->GetName(), *TagChanges.Added.ToStringSimple(), *TagChanges.Removed.ToStringSimple());
	return ASC->ApplyGameplayEffectToSelf(TagsEffect, 1.0f, EffectContext);
}

void UUseGASUtils::GetActorGameplayTags(AActor const* Actor, FGameplayTagContainer& OutGameplayTags)
{
	OutGameplayTags.AppendTags(GetGameplayTags(Actor));
}

FGameplayTagContainer UUseGASUtils::GetGameplayTags(UObject const* Object)
{
	FGameplayTagContainer Tags;
	if (auto const* AsTagInterface = Cast<IGameplayTagAssetInterface>(Object))
	{
		AsTagInterface->GetOwnedGameplayTags(Tags);
	}
	return MoveTemp(Tags);
}

TArray<FGameplayAttribute> UUseGASUtils::K2_ListAttributes(TSubclassOf<UAttributeSet> const AttributeSet)
{
	auto Attributes = TArray<FGameplayAttribute>();
	UAttributeSet::GetAttributesFromSetClass(AttributeSet, Attributes);
	return MoveTemp(Attributes);
}

template <typename T>
TArray<FGameplayAttribute> UUseGASUtils::ListAttributes()
{
	return K2_ListAttributes(T::StaticClass());
}

TArray<UAttributeSet*> UUseGASUtils::FindAttributeSets(AActor const* Target)
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
