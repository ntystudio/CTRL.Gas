// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UseGASAbilitySystemComponent.generated.h"

class UUseGASAbility;

using FUseGASEffectSpecFactoryFn = TFunction<FGameplayEffectSpecHandle(FGameplayEffectSpecHandle)>;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class USEGAS_API UUseGASAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Find the AbilitySystemComponent on an Actor, ActorComponent, or UObject (via TypedOuter).
	static ThisClass* Get(UObject const* SourceObject, bool bWarnIfNotFound = true);
	static ThisClass* GetChecked(UObject const* SourceObject);
	static ThisClass* GetEnsured(UObject const* SourceObject);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static TArray<UAttributeSet*> FindAttributes(AActor const* Target);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static TArray<FGameplayAttribute> ListAttributes(TSubclassOf<UAttributeSet> AttributeSet);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 DefaultLevel = 1; // Default level for the character, must >= 1

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTagUpdated, const FGameplayTag&, Tag, bool, TagExists);

	UPROPERTY(BlueprintAssignable, DisplayName = "On Tag Updated")
	FOnTagUpdated OnTagUpdatedDelegate;

	FActiveGameplayEffectHandle ApplySpecToSelf(UObject const* InSourceObject, TSubclassOf<UGameplayEffect> EffectClass, TOptional<FUseGASEffectSpecFactoryFn> SpecFactory = {});

	bool CanApplyAttributeModifiers(FGameplayEffectSpecHandle const& Spec) const;

	UFUNCTION(BlueprintCallable)
	void RemoveActiveEffectsForHandles(TArray<FActiveGameplayEffectHandle>& Handles);

	virtual int32 GetLevel() const { return DefaultLevel; }
	virtual int32 GetItemLevel(float InLevel) const;
	virtual int32 GetItemLevel(int32 InLevel) const;
	virtual void OnTagUpdated(FGameplayTag const& Tag, bool TagExists) override;

	//~ Ability Input Handling ~//
	typedef TFunctionRef<bool(UUseGASAbility const* Ability, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
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
