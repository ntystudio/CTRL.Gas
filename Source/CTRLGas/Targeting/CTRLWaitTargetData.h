#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "Abilities/GameplayAbilityTargetActor.h"
#include "Abilities/Tasks/AbilityTask.h"

#include "Templates/SubclassOf.h"

#include "UObject/ObjectMacros.h"

#include "CTRLWaitTargetData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCTRLWaitTargetDataDelegate, const FGameplayAbilityTargetDataHandle&, Data);

/**
 * Wait for targeting actor (spawned from parameter) to provide data. Can be set not to end upon outputting data. Can be ended by task name.
 * If bDestroyTargetActor is true, the target actor will be destroyed after the task is finished, like normal WaitTargetData task.
 * If bDestroyTargetActor is false, the target actor will not be destroyed, and can be reused for the next task.
 */
UCLASS()
class CTRLGAS_API UCTRLWaitTargetDataTask : public UAbilityTask
{
	GENERATED_UCLASS_BODY()
	UPROPERTY(BlueprintAssignable)
	FCTRLWaitTargetDataDelegate ValidData;

	UPROPERTY(BlueprintAssignable)
	FCTRLWaitTargetDataDelegate Cancelled;
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCTRLWaitTargetDataTargetActor, AGameplayAbilityTargetActor*, TargetActor);
	
	UPROPERTY(BlueprintAssignable)
	FCTRLWaitTargetDataTargetActor TargetActorCreated;

	UPROPERTY(BlueprintReadWrite)
	bool bDestroyTargetActor = false;

	UFUNCTION()
	virtual void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);

	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelledCallback();

	UFUNCTION()
	virtual void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	virtual void OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data);

	/** Spawns target actor and waits for it to return valid data or to be canceled. */
	UFUNCTION(BlueprintCallable, meta=(HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms="Instigator"), Category="CTRL|Gas|Tasks")
	static UCTRLWaitTargetDataTask* WaitTargetDataUsingReusableActor(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		TEnumAsByte<EGameplayTargetingConfirmation::Type> InConfirmationType,
		AGameplayAbilityTargetActor* InTargetActor,
		bool bInDestroyTargetActor = false
	);

	// /** Uses specified target actor and waits for it to return valid data or to be canceled. */
	// UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	// static UAbilityTask_WaitTargetData* WaitTargetDataUsingActor(UGameplayAbility* OwningAbility, FName TaskInstanceName, TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType, AGameplayAbilityTargetActor* TargetActor);

	virtual void Activate() override;

	// UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Abilities")
	// virtual bool BeginSpawningActor(UGameplayAbility* OwningAbility, TSubclassOf<AGameplayAbilityTargetActor> Class, AGameplayAbilityTargetActor*& SpawnedActor);
	//
	// UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Abilities")
	// virtual void FinishSpawningActor(UGameplayAbility* OwningAbility, AGameplayAbilityTargetActor* SpawnedActor);

	/** Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true. */
	virtual void ExternalConfirm(bool bEndTask) override;

	/** Called when the ability is asked to cancel from an outside node. What this means depends on the individual task. By default, this does nothing other than ending the task. */
	virtual void ExternalCancel() override;

protected:
	// virtual bool ShouldSpawnTargetActor() const;
	virtual void InitializeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const;
	virtual void FinalizeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const;

	virtual void RegisterTargetDataCallbacks();

	virtual void OnDestroy(bool AbilityEnded) override;

	virtual bool ShouldReplicateDataToServer() const;

protected:
	// UPROPERTY(BlueprintReadOnly)
	// TSubclassOf<AGameplayAbilityTargetActor> TargetClass;

	/** The TargetActor that we spawned */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AGameplayAbilityTargetActor> TargetActor;

	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType;

	FDelegateHandle OnTargetDataReplicatedCallbackDelegateHandle;
};

/**
*	Requirements for using Begin/Finish SpawningActor functionality:
*		-Have a parameters named 'Class' in your Proxy factor function (E.g., WaitTargetdata)
*		-Have a function named BeginSpawningActor w/ the same Class parameter
*			-This function should spawn the actor with SpawnActorDeferred and return true/false if it spawned something.
*		-Have a function named FinishSpawningActor w/ an AActor* of the class you spawned
*			-This function *must* call ExecuteConstruction + PostActorConstruction
*/
