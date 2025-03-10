﻿// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

#include "CTRLWaitTaskEnd.generated.h"

/**
 * 
 */
UCLASS(DisplayName="Wait for Task End [CTRL]", Category="CTRL|Gas|Tasks")
class CTRLGAS_API UCTRLWaitTaskEnd : public UAbilityTask
{
	GENERATED_BODY()

public:
	UCTRLWaitTaskEnd();
	UFUNCTION(BlueprintCallable, DisplayName="Wait for Task End [CTRL]", Category="CTRL|Gas|Tasks", meta=(HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms="Instigator"))
	static UCTRLWaitTaskEnd* WaitTaskEnd(UGameplayAbility* OwningAbility, FName TaskInstanceName, UGameplayTask* AsyncTask);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<UGameplayTask> AsyncTask;

	virtual void TickTask(float DeltaTime) override;

	virtual void Activate() override;
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
	bool IsDone() const;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTaskEnd);

	UPROPERTY(BlueprintAssignable)
	FOnTaskEnd OnTaskEnd;
};
