// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask.h"

#include "Animation/AnimNotifies/AnimNotify.h"

#include "CTRLGas/Tasks/CTRLGasAbilityTask.h"

#include "CTRLGasWaitAbilityCooldown.generated.h"

/**
 * Wait for an animation notify to be received.
 */
UCLASS(DisplayName="Wait for Ability Cooldown [CTRLGas]")
class CTRLGAS_API UCTRLGasWaitAbilityCooldown : public UCTRLGasAbilityTask
{
	GENERATED_BODY()

public:
	UCTRLGasWaitAbilityCooldown(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUGWaitForAbilityCooldown);

	UPROPERTY(BlueprintAssignable)
	FUGWaitForAbilityCooldown OnCooldownStart;

	UPROPERTY(BlueprintAssignable)
	FUGWaitForAbilityCooldown OnCooldownEnd;

	UPROPERTY(BlueprintReadOnly)
	bool bCooldownStarted = false;

	UPROPERTY(BlueprintReadOnly)
	bool bCheckForAlreadyCooledDown = false;

	UFUNCTION(
		BlueprintCallable,
		Category="Ability|Tasks",
		meta = (
			DisplayName="Wait for Ability Cooldown [CTRLGas]",
			HidePin = "OwningAbility",
			DefaultToSelf = "OwningAbility",
			BlueprintInternalUseOnly = "true"
		)
	)
	static UCTRLGasWaitAbilityCooldown* WaitForAbilityCooldown(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool bInCheckForAlreadyCooledDown);

	virtual void TickTask(float DeltaTime) override;
	bool IsCoolingDown() const;
	virtual void Activate() override;
};
