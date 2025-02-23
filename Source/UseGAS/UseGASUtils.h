// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"

#include "Abilities/GameplayAbility.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "UseGASUtils.generated.h"

using FUseGASEffectFactoryFn = TFunction<UGameplayEffect*(UGameplayEffect*)>;
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
	UFUNCTION(BlueprintCallable)
	static FActiveGameplayEffectHandle AddGameplayTagsEffect(UAbilitySystemComponent* ASC, FGameplayTagContainer const& Tags, UObject* SourceObject);
	UFUNCTION(BlueprintCallable)
	static FActiveGameplayEffectHandle AddInheritedGameplayTagsEffect(UAbilitySystemComponent* ASC, FInheritedTagContainer const& TagChanges, UObject* SourceObject);

	UFUNCTION(BlueprintCallable)
	static FActiveGameplayEffectHandle K2_AddInheritedGameplayTagsEffectDuration(
		UAbilitySystemComponent* ASC,
		FInheritedTagContainer const& TagChanges,
		float Duration,
		UObject* SourceObject
	);

	// Convenience function for making a new GameplayEffect with the given name and factory function.
	// Uses the following defaults (override as needed in the factory fn):
	// * DurationPolicy = EGameplayEffectDurationType::Infinite;
	// * StackingType = EGameplayEffectStackingType::AggregateByTarget;
	// * StackLimitCount = 1;
	static UGameplayEffect* MakeEffect(UAbilitySystemComponent* ASC, FName const& BaseName, FUseGASEffectFactoryFn Factory = nullptr);

	static FActiveGameplayEffectHandle AddInheritedGameplayTagsEffectDuration(
		UAbilitySystemComponent* ASC,
		FInheritedTagContainer const& TagChanges,
		FGameplayEffectModifierMagnitude Duration,
		UObject const* SourceObject
	);
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(DefaultToSelf="Actor"))
	void GetActorGameplayTags(AActor const* Actor, FGameplayTagContainer& OutGameplayTags);
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(DefaultToSelf="Object"))
	static FGameplayTagContainer GetGameplayTags(UObject const* Object);

	UFUNCTION(BlueprintCallable, meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool IsValidAbilitySpec(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle);

	UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Ability"))
	static void AssignAbilitySetByCallerMagnitude(UGameplayAbility* Ability, FGameplayTag MagnitudeTag, double MagnitudeValue);

	static FPredictionKey GetOriginalPredictionKey(FGameplayAbilitySpec const& Spec);


	
	
	
	// Find all AttributeSets on Actor
	// When the ASC is initialized, it will automatically find and add all AttributeSets
	// on the owner actor e.g. PlayerState
	// If you change who owns the ASC, you will normally need to manually add the AttributeSets or move
	// them to the owner.
	// Instead, you can use this function to find them automatically so your ASC init code remains basically the same.
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(DefaultToSelf="Target"))
	static TArray<UAttributeSet*> FindAttributeSets(AActor const* Target);

	// Lists all attributes in an AttributeSet class
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "List Attributes")
	static TArray<FGameplayAttribute> K2_ListAttributes(TSubclassOf<UAttributeSet> AttributeSet);

	// templated version of ListAttributes
	template <typename T>
	static TArray<FGameplayAttribute> ListAttributes();
};
