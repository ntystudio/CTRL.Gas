// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"

#include "Abilities/GameplayAbility.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "UseGASUtils.generated.h"

/**
 * 
 */
UCLASS()
class USEGAS_API UUseGASUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(ExpandBoolAsExecs="bSuccess"), DisplayName="Get Effect Spec")
	static FGameplayEffectSpec K2_GetEffectSpec(FGameplayEffectSpecHandle const& Handle, bool& bSuccess);
	static TOptional<TSharedPtr<FGameplayEffectSpec>> GetEffectSpec(FGameplayEffectSpecHandle const& Handle);

	UFUNCTION(BlueprintCallable, DisplayName="Get Ability CDO", meta=(ExpandBoolAsExecs="bSuccess", DeterminesOutputType="Class"))
	static UGameplayAbility* K2_GetAbilityCDO(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle, TSubclassOf<UGameplayAbility> Class, bool& bSuccess);
	
	UFUNCTION(BlueprintCallable, DisplayName="Get Ability Primary Instance", meta=(ExpandBoolAsExecs="bSuccess", DeterminesOutputType="Class"))
	static UGameplayAbility* K2_GetAbilityPrimaryInstance(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle, TSubclassOf<UGameplayAbility> Class, bool& bSuccess);

	UFUNCTION(BlueprintCallable, DisplayName="Get Ability Spec Handle", meta=(ExpandBoolAsExecs="bSuccess", DefaultToSelf="Ability"))
	static FGameplayAbilitySpecHandle K2_GetAbilitySpecHandle(UGameplayAbility const* Ability, bool& bSuccess);
	static FGameplayAbilitySpecHandle GetAbilitySpecHandle(UGameplayAbility const* Ability);

	UFUNCTION(BlueprintCallable, meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool IsValidAbilitySpec(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle);
	
	UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Ability"))
	static void AssignAbilitySetByCallerMagnitude(UGameplayAbility* Ability, FGameplayTag MagnitudeTag, double MagnitudeValue);

	static FPredictionKey GetOriginalPredictionKey(FGameplayAbilitySpec const& Spec);
};
