// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"

#include "Abilities/GameplayAbility.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "CTRLGasUtils.generated.h"

using FCTRLGasEffectFactoryFn = TFunction<UGameplayEffect*(UGameplayEffect*)>;
/**
 * 
 */
UCLASS(Category="CTRL|Gas", DisplayName="GAS Utils [CTRL]")
class CTRLGAS_API UCTRLGasUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta=(Keywords="Gameplay", ExpandBoolAsExecs="bSuccess"), DisplayName="Get Effect Spec [CTRL]", Category="CTRL|Gas")
	static FGameplayEffectSpec K2_GetEffectSpec(FGameplayEffectSpecHandle const& Handle, bool& bSuccess);
	static TOptional<TSharedPtr<FGameplayEffectSpec>> GetEffectSpec(FGameplayEffectSpecHandle const& Handle);

	UFUNCTION(BlueprintCallable, DisplayName="Get Ability CDO [CTRL]", Category="CTRL|Gas", meta=(Keywords="Gameplay", ExpandBoolAsExecs="bSuccess", DeterminesOutputType="Class"))
	static UGameplayAbility* K2_GetAbilityCDO(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle, TSubclassOf<UGameplayAbility> Class, bool& bSuccess);

	UFUNCTION(BlueprintCallable, DisplayName="Get Ability Primary Instance [CTRL]", Category="CTRL|Gas", meta=(Keywords="Gameplay", ExpandBoolAsExecs="bSuccess", DeterminesOutputType="Class"))
	static UGameplayAbility* K2_GetAbilityPrimaryInstance(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle, TSubclassOf<UGameplayAbility> Class, bool& bSuccess);

	UFUNCTION(BlueprintCallable, DisplayName="Get Ability Spec Handle [CTRL]", Category="CTRL|Gas", meta=(Keywords="Gameplay", ExpandBoolAsExecs="bSuccess", DefaultToSelf="Ability"))
	static FGameplayAbilitySpecHandle K2_GetAbilitySpecHandle(UGameplayAbility const* Ability, bool& bSuccess);
	static FGameplayAbilitySpecHandle GetAbilitySpecHandle(UGameplayAbility const* Ability);
	
	UFUNCTION(BlueprintCallable, DisplayName="Add Gameplay Tags Effect [CTRL]", Category="CTRL|Gas")
	static FActiveGameplayEffectHandle AddGameplayTagsEffect(UAbilitySystemComponent* ASC, FGameplayTagContainer const& Tags, UObject* SourceObject);
	
	UFUNCTION(BlueprintCallable, DisplayName="Add Inherited Gameplay Tags Effect [CTRL]", Category="CTRL|Gas")
	static FActiveGameplayEffectHandle AddInheritedGameplayTagsEffect(UAbilitySystemComponent* ASC, FInheritedTagContainer const& TagChanges, UObject* SourceObject);

	UFUNCTION(BlueprintCallable, DisplayName="Add Inherited Gameplay Tags Effect for Duration [CTRL]", Category="CTRL|Gas", meta=(Keywords="Time Timer Delay"))
	static FActiveGameplayEffectHandle K2_AddInheritedGameplayTagsEffectDuration(
		UAbilitySystemComponent* ASC,
		FInheritedTagContainer const& TagChanges,
		float Duration,
		UObject* SourceObject
	);
	static FActiveGameplayEffectHandle AddInheritedGameplayTagsEffectDuration(
		UAbilitySystemComponent* ASC,
		FInheritedTagContainer const& TagChanges,
		FGameplayEffectModifierMagnitude Duration,
		UObject const* SourceObject
	);

	// Convenience function for making a new GameplayEffect with the given name and factory function.
	// Uses the following defaults (override as needed in the factory fn):
	// * DurationPolicy = EGameplayEffectDurationType::Infinite;
	// * StackingType = EGameplayEffectStackingType::AggregateByTarget;
	// * StackLimitCount = 1;
	static UGameplayEffect* MakeEffect(UAbilitySystemComponent* ASC, FName const& BaseName, FCTRLGasEffectFactoryFn Factory = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(DefaultToSelf="Actor", Keywords="Container"), DisplayName="Get Actor Gameplay Tags [CTRL]", Category="CTRL|Gas")
	void GetActorGameplayTags(AActor const* Actor, FGameplayTagContainer& OutGameplayTags);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(DefaultToSelf="Object", Keywords="Container"), DisplayName="Get Gameplay Tags [CTRL]", Category="CTRL|Gas")
	static FGameplayTagContainer GetGameplayTags(UObject const* Object);

	UFUNCTION(BlueprintCallable, meta=(ExpandBoolAsExecs="ReturnValue", Keywords="Gameplay"), DisplayName="Is Valid Ability Spec [CTRL]", Category="CTRL|Gas")
	static bool IsValidAbilitySpec(UAbilitySystemComponent const* ASC, FGameplayAbilitySpecHandle const& Handle);

	UFUNCTION(BlueprintCallable, meta=(DefaultToSelf="Ability", Keywords="Gameplay SetByCaller"), DisplayName="Get Ability Set By Caller Magnitude [CTRL]", Category="CTRL|Gas")
	static void AssignAbilitySetByCallerMagnitude(UGameplayAbility* Ability, FGameplayTag MagnitudeTag, double MagnitudeValue);

	static FPredictionKey GetOriginalPredictionKey(FGameplayAbilitySpec const& Spec);

	// Find all AttributeSets on Actor
	// When the ASC is initialized, it will automatically find and add all AttributeSets
	// on the owner actor e.g. PlayerState
	// If you change who owns the ASC, you will normally need to manually add the AttributeSets or move
	// them to the owner.
	// Instead, you can use this function to find them automatically so your ASC init code remains basically the same.
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(DefaultToSelf="Target", Keywords="Gameplay Attr"), DisplayName="Find Attribute Sets [CTRL]", Category="CTRL|Gas")
	static TArray<UAttributeSet*> FindAttributeSets(AActor const* Target);

	// Lists all attributes in an AttributeSet class
	UFUNCTION(BlueprintCallable, BlueprintPure, DisplayName = "List Attributes [CTRL]", Category = "CTRL|Gas", meta = (Keywords = "Gameplay Attr"))
	static TArray<FGameplayAttribute> K2_ListAttributes(TSubclassOf<UAttributeSet> AttributeSet);

	// templated version of ListAttributes
	template <typename T>
	static TArray<FGameplayAttribute> ListAttributes();
};
