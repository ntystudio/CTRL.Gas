// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"

#include "CTRLGas/Tasks/CTRLGasAbilityTask.h"

#include "CTRLGasWaitInputPressRelease.generated.h"

/**
 * 
 */
UCLASS()
class CTRLGAS_API UCTRLGasWaitInputPressRelease : public UCTRLGasAbilityTask
{
	GENERATED_BODY()

public:
	/** Wait until the user presses the input button for this ability's activation. Returns time this node spent waiting for the press. Will return 0 if input was already down. */
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UCTRLGasWaitInputPressRelease* WaitInputPressRelease(UGameplayAbility* OwningAbility, bool bInTestAlreadyPressed = true, bool bInTestAlreadyReleased = false, bool bInEndOnPressed = true, bool bInEndOnReleased = true);

	UPROPERTY(BlueprintAssignable)
	FInputPressDelegate OnPress;

	UPROPERTY(BlueprintAssignable)
	FInputPressDelegate OnRelease;

	UFUNCTION()
	void OnPressCallback();

	UFUNCTION()
	void OnReleaseCallback();

	virtual void Activate() override;

protected:
	float StartTime = 0.0f;
	bool bTestAlreadyPressed = true;
	bool bTestAlreadyReleased = false;
	bool bEndOnPressed = true;
	bool bEndOnReleased = true;
	FDelegateHandle ReleasedDelegateHandle;
	FDelegateHandle PressedDelegateHandle;
};
