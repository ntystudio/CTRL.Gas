// SPDX-FileCopyrightText: 2025 NTY.studio

#include "UseGAS/UseGASUtils.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "GameplayPrediction.h"

void UUseGASUtils::AssignAbilitySetByCallerMagnitude(UGameplayAbility* Ability, FGameplayTag const MagnitudeTag, double const MagnitudeValue) {
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
