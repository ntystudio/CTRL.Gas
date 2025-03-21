﻿// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "CTRLAbilitySystemComponent.generated.h"

class UCTRLGasAbility;

using FCTRLGasEffectSpecFactoryFn = TFunction<FGameplayEffectSpecHandle(FGameplayEffectSpecHandle)>;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName="Ability System Component [CTRL]", Category="CTRL|Gas")
class CTRLGAS_API UCTRLAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Find the AbilitySystemComponent on an Actor, ActorComponent, or UObject (via TypedOuter).
	UFUNCTION(DisplayName = "Get CTRL ASC [CTRL]", Category="CTRL|Gas", meta=(Keywords="Gameplay Ability System Component"))
	static UCTRLAbilitySystemComponent* GetASC(UObject const* SourceObject, bool bWarnIfNotFound = true);
	UFUNCTION(DisplayName = "Get CTRL ASC Checked [CTRL]", Category="CTRL|Gas", meta=(Keywords="Gameplay Ability System Component"))
	static UCTRLAbilitySystemComponent* GetASCChecked(UObject const* SourceObject);
	UFUNCTION(DisplayName = "Get CTRL ASC Ensured [CTRL]", Category="CTRL|Gas", meta=(Keywords="Gameplay Ability System Component"))
	static UCTRLAbilitySystemComponent* GetASCEnsured(UObject const* SourceObject);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	int32 DefaultLevel = 1; // Default level for the character, must >= 1

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTagUpdated, const FGameplayTag&, Tag, bool, TagExists);

	UPROPERTY(BlueprintAssignable, DisplayName = "On Tag Updated")
	FOnTagUpdated OnTagUpdatedDelegate;

	UE_DEPRECATED(5.5, "Use MakeEffectSpec instead")
	FGameplayEffectSpecHandle MakeSpec(TSubclassOf<UGameplayEffect> const& GameplayEffectClass, TOptional<float> const& Level = {}, TOptional<FGameplayEffectContextHandle> const& Context = {}) const;
	FGameplayEffectSpecHandle MakeEffectSpec(TSubclassOf<UGameplayEffect> const& GameplayEffectClass, TOptional<float> Level = {}, TOptional<FGameplayEffectContextHandle> Context = {}) const;
	FGameplayAbilitySpec MakeAbilitySpec(TSubclassOf<UGameplayAbility> const& GameplayAbilityClass, TOptional<float> const& Level = {}, UObject* Source = nullptr) const;
	// ApplyGameplayEffectSpecToSelf with a factory function to modify the spec before applying.
	FActiveGameplayEffectHandle ApplySpecToSelf(UObject const* InSourceObject, TSubclassOf<UGameplayEffect> EffectClass, FCTRLGasEffectSpecFactoryFn SpecFactory = nullptr);

	// e.g. for checking cost before applying
	// Adapted from FActiveGameplayEffectsContainer::CanApplyAttributeModifiers, but takes a spec handle
	// instead of gameplay effect subclass.
	// This allows our effect to use the same logic as the actual effect, in particular
	// it enables setting a different source object for the effect, but also allows
	// other customizations like custom context or level so it can exactly match the setup
	// when we apply the effect.
	// Stock CanApplyAttributeModifiers/CheckCost only supports source == target.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="CTRL|Gas", meta=(Keywords="Gameplay Effect"), DisplayName="Can Apply Attribute Modifiers [CTRL]")
	bool CanApplyAttributeModifiers(FGameplayEffectSpecHandle const& SpecHandle) const;

	// Closer to FActiveGameplayEffectsContainer::CanApplyAttributeModifiers but takes a
	// spec factory function to modify the spec before checking.
	bool CanApplyAttributeModifiers(TSubclassOf<UGameplayEffect> const& EffectClass, FCTRLGasEffectSpecFactoryFn const& SpecFactory = nullptr) const;

	// Remove all active effects with the given handles.
	// Resets the passed-in array.
	UFUNCTION(BlueprintCallable, Category="CTRL|Gas")
	void RemoveActiveEffectsForHandles(TArray<FActiveGameplayEffectHandle>& Handles);

	virtual int32 GetLevel() const { return DefaultLevel; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="CTRL|Gas", DisplayName="Get ASC Level [CTRL]", meta=(Keywords="Effect Spec Ability"))
	int32 K2_GetLevel() const;

	// Return the level or DefaultLevel if InLevel is -1.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="CTRL|Gas", DisplayName="Get Level or Default [CTRL]", meta=(Keywords="Effect Spec Ability"))
	int32 K2_GetLevelOrDefault(float InLevel = -1) const;

	virtual int32 GetLevelOrDefault(float InLevel) const;
	virtual int32 GetLevelOrDefault(int32 InLevel) const;
	virtual int32 GetLevelOrDefault(TOptional<int32> InLevel) const;
	virtual int32 GetLevelOrDefault(TOptional<float> InLevel) const;
	virtual void OnTagUpdated(FGameplayTag const& Tag, bool TagExists) override;

	//~ Ability Input Handling ~//
	typedef TFunctionRef<bool(UCTRLGasAbility const* Ability, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
	void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc, bool bReplicateCancelAbility);
	void CancelInputActivatedAbilities(bool bReplicateCancelAbility);
	void ClearAbilityInput();
	void ProcessAbilityInput(float DeltaTime, bool bGamePaused);
	void OnAbilityInputTagPressed(FGameplayTag const& InputTag);
	void OnAbilityInputTagReleased(FGameplayTag const& InputTag);

protected:
	// Handles to abilities that had their input pressed this frame.
	TArray<FGameplayAbilitySpecHandle> InputPressedSpecHandles;

	// Handles to abilities that had their input released this frame.
	TArray<FGameplayAbilitySpecHandle> InputReleasedSpecHandles;

	// Handles to abilities that have their input held.
	TArray<FGameplayAbilitySpecHandle> InputHeldSpecHandles;
};
