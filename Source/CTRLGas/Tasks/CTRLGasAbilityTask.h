// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask.h"

#include "CTRLGasAbilityTask.generated.h"

#define END_WITH_ERROR(Verbosity, ...) \
([&]() -> void { \
	UE_LOG(LogTemp, Verbosity, ##__VA_ARGS__); \
	if (!ShouldBroadcastAbilityTaskDelegates()) { return; } \
	OnError.Broadcast(); \
	EndTask(); \
}())

class UCTRLAbilitySystemComponent;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, DisplayName="Ability Task [CTRL]", Category="CTRL|Gas|Tasks")
class CTRLGAS_API UCTRLGasAbilityTask : public UAbilityTask
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUGAbilityTaskDelegate);

	// Triggered when the task is activated.
	UPROPERTY(BlueprintAssignable)
	FUGAbilityTaskDelegate OnActivated;

	// Triggered when the task is completed, successfully or not.
	UPROPERTY(BlueprintAssignable)
	FUGAbilityTaskDelegate OnCompleted;

	// Triggered when the task fails.
	UPROPERTY(BlueprintAssignable)
	FUGAbilityTaskDelegate OnError;

	// Get the CTRL Gameplay Ability System Component from the Ability's owner actor.
	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(Keywords="Gameplay Ability System Component"))
	UCTRLAbilitySystemComponent* GetASC() const;

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
};
