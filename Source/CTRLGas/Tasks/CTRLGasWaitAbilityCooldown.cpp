// SPDX-FileCopyrightText: 2025 NTY.studio

#include "CTRLGas/Tasks/CTRLGasWaitAbilityCooldown.h"

#include "AbilitySystemComponent.h"

UCTRLGasWaitAbilityCooldown::UCTRLGasWaitAbilityCooldown(FObjectInitializer const& ObjectInitializer): Super(ObjectInitializer)
{
	bTickingTask = true;
}

UCTRLGasWaitAbilityCooldown* UCTRLGasWaitAbilityCooldown::WaitForAbilityCooldown(UGameplayAbility* OwningAbility, FName const TaskInstanceName, bool const bInCheckForAlreadyCooledDown)
{
	auto const Task = NewAbilityTask<UCTRLGasWaitAbilityCooldown>(OwningAbility, TaskInstanceName);
	Task->bCheckForAlreadyCooledDown = bInCheckForAlreadyCooledDown;
	return Task;
}

void UCTRLGasWaitAbilityCooldown::TickTask(float const DeltaTime)
{
	Super::TickTask(DeltaTime);
	if (!ShouldBroadcastAbilityTaskDelegates()) return;
	if (!IsValid(Ability) || !Ability->IsActive()) return;
	if (!bCooldownStarted && IsCoolingDown())
	{
		bCooldownStarted = true;
		OnCooldownStart.Broadcast();
	}
	else
	{
		if (bCooldownStarted && !IsCoolingDown())
		{
			OnCooldownEnd.Broadcast();
			EndTask();
		}
	}
}

bool UCTRLGasWaitAbilityCooldown::IsCoolingDown() const
{
	if (!IsValid(Ability) || !Ability->IsActive()) return false;
	return Ability->GetCooldownTimeRemaining() > 0.0f;
}

void UCTRLGasWaitAbilityCooldown::Activate()
{
	Super::Activate();
	if (!ShouldBroadcastAbilityTaskDelegates()) { return; }
	if (bCheckForAlreadyCooledDown && Ability->IsActive() && !IsCoolingDown())
	{
		OnCooldownEnd.Broadcast();
		EndTask();
	}
}
