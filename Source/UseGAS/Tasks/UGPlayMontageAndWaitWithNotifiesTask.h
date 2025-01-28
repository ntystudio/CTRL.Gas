// // SPDX-FileCopyrightText: 2025 NTY.studio
// #pragma once
//
// #include "CoreMinimal.h"
//
// #include "Abilities/Tasks/AbilityTask.h"
//
// #include "Animation/AnimInstance.h"
//
// #include "Kismet/BlueprintAsyncActionBase.h"
//
// #include "UObject/ObjectMacros.h"
// #include "UGPlayMontageAndWaitWithNotifiesTask.generated.h"
//
// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMontageWaitWithNotifiesSimpleDelegate);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMontageWaitWithNotifiesNotifyDelegate, FName, NotifyName);
//
// /** Ability task to simply play a montage. Many games will want to make a modified version of this task that looks for game-specific events */
// UCLASS()
// class USEGAS_API UUGPlayMontageAndWaitWithNotifiesTask : public UAbilityTask
// {
// 	GENERATED_BODY()
//
// public:
// 	UPROPERTY(BlueprintReadOnly)
// 	TObjectPtr<UAnimInstance> AnimInstance;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnCompleted;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnBlendedIn;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnBlendOut;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnInterrupted;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnCancelled;
// 	
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesNotifyDelegate OnNotifyBegin;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesNotifyDelegate OnNotifyEnd;
//
// 	UPROPERTY(BlueprintReadOnly)
// 	float MontageLength = 0.f;
//
// 	UFUNCTION()
// 	void OnMontageBlendedIn(UAnimMontage* Montage);
//
// 	bool IsPlayingThisMontage() const;
// 	UFUNCTION()
// 	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);
//
// 	UE_DEPRECATED(5.3, "Please use OnGameplayAbilityCancelled instead. This function naming implied the Montage was already interrupted (instead, we are about to interrupt it).")
// 	UFUNCTION()
// 	void OnMontageInterrupted();
//
// 	/** Callback function for when the owning Gameplay Ability is cancelled */
// 	UFUNCTION()
// 	void OnGameplayAbilityCancelled();
//
// 	UFUNCTION()
// 	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
//
// 	/** 
// 	 * Start playing an animation montage on the avatar actor and wait for it to finish
// 	 * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
// 	 * On normal execution, OnBlendOut is called when the montage is blending out, and OnCompleted when it is completely done playing
// 	 * OnInterrupted is called if another montage overwrites this, and OnCancelled is called if the ability or task is cancelled
// 	 *
// 	 * @param TaskInstanceName Set to override the name of this task, for later querying
// 	 * @param MontageToPlay The montage to play on the character
// 	 * @param Rate Change to play the montage faster or slower
// 	 * @param StartSection If not empty, named montage section to start from
// 	 * @param bStopWhenAbilityEnds If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
// 	 * @param AnimRootMotionTranslationScale Change to modify size of root motion or set to 0 to block it entirely
// 	 * @param StartTimeSeconds Starting time offset in montage, this will be overridden by StartSection if that is also set
// 	 * @param bAllowInterruptAfterBlendOut If true, you can receive OnInterrupted after an OnBlendOut started (otherwise OnInterrupted will not fire when interrupted, but you will not get OnComplete).
// 	 */
// 	UFUNCTION(
// 		BlueprintCallable,
// 		Category="Ability|Tasks",
// 		meta = (DisplayName="Play Montage and Wait (with Notifies)",
// 			HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE")
// 	)
// 	static UAbilityTask_PlayMontageAndWaitWithNotifies* CreatePlayMontageAndWaitWithNotifiesProxy(
// 		UGameplayAbility* OwningAbility,
// 		FName TaskInstanceName,
// 		UAnimMontage* MontageToPlay,
// 		float Rate = 1.f,
// 		FName StartSection = NAME_None,
// 		bool bStopWhenAbilityEnds = true,
// 		float AnimRootMotionTranslationScale = 1.f,
// 		float StartTimeSeconds = 0.f,
// 		bool bAllowInterruptAfterBlendOut = false
// 	);
//
// 	virtual void Activate() override;
//
// 	/** Called when the ability is asked to cancel from an outside node. What this means depends on the individual task. By default, this does nothing other than ending the task. */
// 	virtual void ExternalCancel() override;
//
// 	virtual FString GetDebugString() const override;
//
// protected:
// 	UFUNCTION()
// 	void OnNotifyBeginReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
// 	UFUNCTION()
// 	void OnNotifyEndReceived(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
// 	virtual void OnDestroy(bool AbilityEnded) override;
// 	void ClearDelegates();
//
// 	/** Checks if the ability is playing a montage and stops that montage, returns true if a montage was stopped, false if not. */
// 	bool StopPlayingMontage();
//
// 	FOnMontageBlendedInEnded BlendedInDelegate;
// 	FOnMontageBlendingOutStarted BlendingOutDelegate;
// 	FOnMontageEnded MontageEndedDelegate;
// 	FDelegateHandle InterruptedHandle;
//
// 	UPROPERTY()
// 	TObjectPtr<UAnimMontage> MontageToPlay;
//
// 	UPROPERTY()
// 	float Rate;
//
// 	UPROPERTY()
// 	FName StartSection;
//
// 	UPROPERTY()
// 	float AnimRootMotionTranslationScale;
//
// 	UPROPERTY()
// 	float StartTimeSeconds;
//
// 	UPROPERTY()
// 	bool bStopWhenAbilityEnds;
//
// 	UPROPERTY()
// 	bool bAllowInterruptAfterBlendOut;
// };
//
// UCLASS()
// class MAGITECH_API UAsyncAction_PlayMontageAndWaitWithNotifies : public UBlueprintAsyncActionBase
// {
// 	GENERATED_BODY()
//
// public:
//
// 	UPROPERTY(BlueprintReadWrite)
// 	TObjectPtr<UAnimMontage> MontageToPlay;
//
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere)
// 	float Rate = 1.f;
//
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere)
// 	FName StartSection;
//
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere)
// 	float AnimRootMotionTranslationScale = 1.f;
//
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere)
// 	float StartTimeSeconds = 0.f;
//
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere)
// 	bool bStopWhenAbilityEnds = true;
//
// 	UPROPERTY(BlueprintReadWrite, EditAnywhere)
// 	bool bAllowInterruptAfterBlendOut = false;
//
// 	UPROPERTY(BlueprintReadOnly, EditAnywhere)
// 	TObjectPtr<UAnimInstance> AnimInstance;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnCompleted;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnBlendedIn;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnBlendOut;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnInterrupted;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesSimpleDelegate OnCancelled;
// 	
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesNotifyDelegate OnNotifyBegin;
//
// 	UPROPERTY(BlueprintAssignable)
// 	FMontageWaitWithNotifiesNotifyDelegate OnNotifyEnd;
//
// 	UPROPERTY(BlueprintReadOnly, EditAnywhere)
// 	float MontageLength = 0.f;
//
// 	UPROPERTY(BlueprintReadOnly, EditAnywhere)
// 	TObjectPtr<UAbilityTask_PlayMontageAndWaitWithNotifies> Task;
// 	
// 	/** 
// 	 * Start playing an animation montage on the avatar actor and wait for it to finish
// 	 * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
// 	 * On normal execution, OnBlendOut is called when the montage is blending out, and OnCompleted when it is completely done playing
// 	 * OnInterrupted is called if another montage overwrites this, and OnCancelled is called if the ability or task is cancelled
// 	 *
// 	 * @param InTaskInstanceName Set to override the name of this task, for later querying
// 	 * @param InMontageToPlay The montage to play on the character
// 	 * @param InRate Change to play the montage faster or slower
// 	 * @param StartSection If not empty, named montage section to start from
// 	 * @param bStopWhenAbilityEnds If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
// 	 * @param AnimRootMotionTranslationScale Change to modify size of root motion or set to 0 to block it entirely
// 	 * @param StartTimeSeconds Starting time offset in montage, this will be overridden by StartSection if that is also set
// 	 * @param bAllowInterruptAfterBlendOut If true, you can receive OnInterrupted after an OnBlendOut started (otherwise OnInterrupted will not fire when interrupted, but you will not get OnComplete).
// 	 */
// 	UFUNCTION(
// 		BlueprintCallable,
// 		Category="Ability|Tasks",
// 		meta = (DisplayName="Async Play Montage and Wait (with Notifies)",
// 			HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE")
// 	)
// 	static UAsyncAction_PlayMontageAndWaitWithNotifies* CreatePlayMontageAndWaitWithNotifiesProxy(
// 		UGameplayAbility* OwningAbility,
// 		FName InTaskInstanceName,
// 		UAnimMontage* InMontageToPlay,
// 		float InRate = 1.f,
// 		FName InStartSection = NAME_None,
// 		bool bInStopWhenAbilityEnds = true,
// 		float InAnimRootMotionTranslationScale = 1.f,
// 		float InStartTimeSeconds = 0.f,
// 		bool bInAllowInterruptAfterBlendOut = false
// 	);
// 	virtual void Activate() override;
// };
