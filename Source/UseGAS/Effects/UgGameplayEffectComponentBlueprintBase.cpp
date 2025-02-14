// SPDX-FileCopyrightText: 2025 NTY.studio

#include "UseGAS/Effects/UgGameplayEffectComponentBlueprintBase.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

#include "Engine/BlueprintGeneratedClass.h"

UAbilitySystemComponent* UUgGameplayEffectComponentBlueprintBase::GetASC(FGameplayEffectSpec const& GESpec)
{
	return GESpec.GetContext().GetInstigatorAbilitySystemComponent();
}

bool UUgGameplayEffectComponentBlueprintBase::K2_CanGameplayEffectApply_Implementation(FGameplayEffectSpec const& GESpec) const
{
	if (auto const ASC = GetASC(GESpec))
	{
		return Super::CanGameplayEffectApply(ASC->GetActiveGameplayEffects(), GESpec);
	}
	return false;
}

UUgGameplayEffectComponentBlueprintBase::UUgGameplayEffectComponentBlueprintBase()
{
	auto ImplementedInBlueprint = [](UFunction const* Func) -> bool
	{
		return Func && ensure(Func->GetOuter()) && Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
	};

	{
		static FName FuncName = FName(TEXT("K2_CanGameplayEffectApply"));
		bHasBlueprintCanGameplayEffectApply = ImplementedInBlueprint(GetClass()->FindFunctionByName(FuncName));
	}
	{
		static FName FuncName = FName(TEXT("K2_OnGameplayEffectExecuted"));
		bHasBlueprintOnGameplayEffectExecuted = ImplementedInBlueprint(GetClass()->FindFunctionByName(FuncName));
	}
	{
		static FName FuncName = FName(TEXT("K2_OnGameplayEffectApplied"));
		bHasBlueprintOnGameplayEffectApplied = ImplementedInBlueprint(GetClass()->FindFunctionByName(FuncName));
	}
}

bool UUgGameplayEffectComponentBlueprintBase::CanGameplayEffectApply(FActiveGameplayEffectsContainer const& ActiveGEContainer, FGameplayEffectSpec const& GESpec) const
{
	if (!Super::CanGameplayEffectApply(ActiveGEContainer, GESpec))
	{
		return false;
	}
	if (bHasBlueprintCanGameplayEffectApply)
	{
		return K2_CanGameplayEffectApply(GESpec);
	}
	return true;
}

void UUgGameplayEffectComponentBlueprintBase::OnGameplayEffectExecuted(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	if (bHasBlueprintOnGameplayEffectExecuted)
	{
		K2_OnGameplayEffectExecuted(GESpec);
	}
	Super::OnGameplayEffectExecuted(ActiveGEContainer, GESpec, PredictionKey);
}

void UUgGameplayEffectComponentBlueprintBase::OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const
{
	if (bHasBlueprintCanGameplayEffectApply)
	{
		K2_OnGameplayEffectApplied(GESpec);
	}
	Super::OnGameplayEffectApplied(ActiveGEContainer, GESpec, PredictionKey);
}
