// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectComponent.h"
#include "UgGameplayEffectComponentBlueprintBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, Abstract, Const=false, DisplayName = "Gameplay Effect Component Blueprint Base [UseGAS]")
class USEGAS_API UUgGameplayEffectComponentBlueprintBase : public UGameplayEffectComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static UAbilitySystemComponent* GetASC(FGameplayEffectSpec const& GESpec);
	UUgGameplayEffectComponentBlueprintBase();

	UFUNCTION(BlueprintNativeEvent)
	bool K2_CanGameplayEffectApply(FGameplayEffectSpec const& GESpec) const;
	bool bHasBlueprintCanGameplayEffectApply;
	UFUNCTION(BlueprintImplementableEvent)
	void K2_OnGameplayEffectExecuted(UPARAM(ref) FGameplayEffectSpec& GESpec) const;
	bool bHasBlueprintOnGameplayEffectExecuted;
	UFUNCTION(BlueprintImplementableEvent)
	void K2_OnGameplayEffectApplied(UPARAM(ref) FGameplayEffectSpec& GESpec) const;
	bool bHasBlueprintOnGameplayEffectApplied;
	virtual bool CanGameplayEffectApply(FActiveGameplayEffectsContainer const& ActiveGEContainer, FGameplayEffectSpec const& GESpec) const override;
	virtual void OnGameplayEffectExecuted(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const override;
	virtual void OnGameplayEffectApplied(FActiveGameplayEffectsContainer& ActiveGEContainer, FGameplayEffectSpec& GESpec, FPredictionKey& PredictionKey) const override;
};
