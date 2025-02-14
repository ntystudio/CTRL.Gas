// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask.h"

#include "Animation/AnimNotifies/AnimNotify.h"

#include "UseGAS/Tasks/UgAbilityTask.h"

#include "UgWaitAbilityCooldown.generated.h"

/**
 * Wait for an animation notify to be received.
 */
UCLASS(DisplayName="Wait for Ability Cooldown [Ug]")
class USEGAS_API UUgWaitAbilityCooldown : public UUgAbilityTask
{
	GENERATED_BODY()

public:
	UUgWaitAbilityCooldown(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());
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
			DisplayName="Wait for Ability Cooldown [Ug]",
			HidePin = "OwningAbility",
			DefaultToSelf = "OwningAbility",
			BlueprintInternalUseOnly = "true"
		)
	)
	static UUgWaitAbilityCooldown* WaitForAbilityCooldown(UGameplayAbility* OwningAbility, FName TaskInstanceName, bool bInCheckForAlreadyCooledDown);

	virtual void TickTask(float DeltaTime) override;
	bool IsCoolingDown() const;
	virtual void Activate() override;
};
