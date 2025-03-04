// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"

#include "Abilities/GameplayAbility.h"

#include "CTRLGas/CTRLGasAbilitySourceInterface.h"

#include "GameFramework/Character.h"

#include "CTRLGasAbility.generated.h"

class ICTRLGasAbilitySourceInterface;
class UCTRLAbilitySystemComponent;
/**
 * ELyraAbilityActivationPolicy
 *
 *	Defines how an ability is meant to activate.
 */
UENUM(BlueprintType)
enum class ELyraAbilityActivationPolicy : uint8
{
	// Try to activate the ability when the input is triggered.
	OnInputTriggered,

	// Continually try to activate the ability while the input is active.
	WhileInputActive,

	// Try to activate the ability when an avatar is assigned.
	OnSpawn
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityEnded, bool, bIsCancelled);

UCLASS(DisplayName="Gameplay Ability [CTRLGas]")
class CTRLGAS_API UCTRLGasAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UCTRLGasAbility(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());
	// Defines how this ability is meant to activate.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ELyraAbilityActivationPolicy ActivationPolicy = ELyraAbilityActivationPolicy::OnInputTriggered;

	// Effects to apply while this ability is active (note ability may not have committed yet)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> OnActivateEffects;

	// Effects to apply when this ability is committed. (i.e. basically like cost, but multiple effects can be applied)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> CommitEffects;

	UPROPERTY(Transient)
	TArray<FActiveGameplayEffectHandle> OnActivateEffectHandles;

	UPROPERTY(Transient)
	TArray<FActiveGameplayEffectHandle> OnCommitEffectHandles;

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UCTRLAbilitySystemComponent* GetASC() const;

	UFUNCTION(BlueprintCallable)
	bool TryActivate(bool bAllowRemoteActivation = true);

	void TryActivateAbilityOnSpawn(FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilitySpec const& Spec) const;

	// Create targeting info using camera (rather than actor's location/viewpoint).
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FGameplayAbilityTargetingLocationInfo MakeTargetingLocationInfoFromCamera();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AController* GetControllerFromActorInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	ACharacter* GetCharacterFromActorInfo() const;

public:
	virtual void ActivateAbility(
		FGameplayAbilitySpecHandle const Handle,
		FGameplayAbilityActorInfo const* ActorInfo,
		FGameplayAbilityActivationInfo const ActivationInfo,
		FGameplayEventData const* TriggerEventData
	) override;

	virtual bool CommitAbility(
		FGameplayAbilitySpecHandle const Handle,
		FGameplayAbilityActorInfo const* ActorInfo,
		FGameplayAbilityActivationInfo const ActivationInfo,
		FGameplayTagContainer* OptionalRelevantTags = nullptr
	) override;

	virtual void EndAbility(
		FGameplayAbilitySpecHandle const Handle,
		FGameplayAbilityActorInfo const* ActorInfo,
		FGameplayAbilityActivationInfo const ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;

	virtual void OnGiveAbility(FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilitySpec const& Spec) override;
	virtual void OnRemoveAbility(FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilitySpec const& Spec) override;
	virtual void ApplyAbilityTagsToGameplayEffectSpec(FGameplayEffectSpec& Spec, FGameplayAbilitySpec* AbilitySpec) const override;
	virtual FGameplayEffectContextHandle MakeEffectContext(FGameplayAbilitySpecHandle Handle, FGameplayAbilityActorInfo const* ActorInfo) const override;
	// ReSharper disable once CppRedefinitionOfDefaultArgumentInOverrideFunction
	virtual FGameplayEffectSpecHandle MakeOutgoingGameplayEffectSpec(FGameplayAbilitySpecHandle const Handle, FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilityActivationInfo const ActivationInfo, TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level = -1) const override;
	// wrapper to use CTRLAbilitySystemComponent's CanApplyAttributeModifiers
	virtual bool CheckCost(FGameplayAbilitySpecHandle const Handle, FGameplayAbilityActorInfo const* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ApplyCost(FGameplayAbilitySpecHandle const Handle, FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilityActivationInfo const ActivationInfo) const override;
	bool CheckCooldown(FGameplayAbilitySpecHandle Handle, FGameplayAbilityActorInfo const* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const;
	void ApplyCooldown(FGameplayAbilitySpecHandle Handle, FGameplayAbilityActorInfo const* ActorInfo, FGameplayAbilityActivationInfo ActivationInfo) const;

public:
	ELyraAbilityActivationPolicy GetActivationPolicy() const { return ActivationPolicy; }

	void GetAbilitySource(
		FGameplayAbilitySpecHandle Handle,
		FGameplayAbilityActorInfo const* ActorInfo,
		float& OutSourceLevel,
		ICTRLGasAbilitySourceInterface const*& OutAbilitySource,
		AActor*& OutEffectCauser
	) const;

protected:
	void ClearOnActivateEffects();
	void ClearOnCommitEffects();
	void ClearEffects();

	// On Ability Added/Given/Granted.
	UFUNCTION(BlueprintImplementableEvent, Category="Ability", DisplayName="OnGiveAbilityEvent", meta=(ScriptName="OnGiveAbilityEvent"))
	void K2_OnGiveAbility();

	// On Ability Removed/Cleared/Revoked.
	UFUNCTION(BlueprintImplementableEvent, Category="Ability", DisplayName="OnRemoveAbilityEvent", meta=(Keywords="Remove Clear Revoke", ScriptName="OnRemoveAbilityEvent"))
	void K2_OnRemoveAbility();
	
	UFUNCTION(BlueprintImplementableEvent, Category="Ability", DisplayName="OnEndAbilityEvent", meta=(ScriptName="OnEndAbilityEvent"))
	void K2_OnEndAbilityEvent(bool bWasCancelled);
};
