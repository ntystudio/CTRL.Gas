// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask.h"

#include "Animation/AnimNotifies/AnimNotify.h"

#include "UseGAS/Tasks/UgAbilityTask.h"

#include "UgWaitForAnimNotifyTask.generated.h"

/**
 * Wait for an animation notify to be received.
 */
UCLASS(DisplayName="Wait for Anim Notify [Ug]")
class USEGAS_API UUgWaitForAnimNotifyTask : public UUgAbilityTask
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUGWaitForAnimNotifyDelegate);

	UPROPERTY()
	TObjectPtr<UAnimMontage> ExpectedMontage;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> CurrentMontage;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAnimInstance> AnimInstance;

	UPROPERTY(BlueprintReadOnly, meta=(GetOptions="GetNotifyNames"))
	FName ListenForNotify;

	UPROPERTY(BlueprintAssignable)
	FUGWaitForAnimNotifyDelegate OnNotifyBegin;

	UPROPERTY(BlueprintAssignable)
	FUGWaitForAnimNotifyDelegate OnNotifyEnd;

	UPROPERTY(BlueprintAssignable)
	FUGWaitForAnimNotifyDelegate OnInterrupted;

	UFUNCTION()
	TArray<FName> GetNotifyNames() const;

	UFUNCTION(
		BlueprintCallable,
		Category="Ability|Tasks",
		meta = (
			DisplayName="Wait for Anim Notify [Ug]",
			HidePin = "OwningAbility",
			DefaultToSelf = "OwningAbility",
			BlueprintInternalUseOnly = "true"
		)
	)
	static UUgWaitForAnimNotifyTask* WaitForAnimNotify(UGameplayAbility* OwningAbility, FName TaskInstanceName, UAnimMontage* InExpectedMontage, UPARAM(meta=(GetOptions="GetNotifyNames")) FName InNotifyName);

	bool IsPlayingExpectedMontage() const;
	bool EnsureIsPlayingExpectedMontage();
	void RemoveBindings() const;

	UFUNCTION()
	void OnNotifyBeginReceived(FName NotifyName, FBranchingPointNotifyPayload const& BranchingPointPayload);

	UFUNCTION()
	void OnNotifyEndReceived(FName NotifyName, FBranchingPointNotifyPayload const& BranchingPointPayload);

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	virtual void Activate() override;
	virtual void BeginDestroy() override;
	virtual void OnDestroy(bool AbilityEnded) override;
};
