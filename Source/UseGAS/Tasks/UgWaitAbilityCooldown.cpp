// SPDX-FileCopyrightText: 2025 NTY.studio

#include "UseGAS/Tasks/UgWaitAbilityCooldown.h"

#include "AbilitySystemComponent.h"

UUgWaitAbilityCooldown::UUgWaitAbilityCooldown(FObjectInitializer const& ObjectInitializer): Super(ObjectInitializer)
{
	bTickingTask = true;
}

UUgWaitAbilityCooldown* UUgWaitAbilityCooldown::WaitForAbilityCooldown(UGameplayAbility* OwningAbility, FName const TaskInstanceName, bool const bInCheckForAlreadyCooledDown)
{
	auto const Task = NewAbilityTask<UUgWaitAbilityCooldown>(OwningAbility, TaskInstanceName);
	Task->bCheckForAlreadyCooledDown = bInCheckForAlreadyCooledDown;
	return Task;
}

void UUgWaitAbilityCooldown::TickTask(float const DeltaTime)
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

bool UUgWaitAbilityCooldown::IsCoolingDown() const
{
	if (!IsValid(Ability) || !Ability->IsActive()) return false;
	return Ability->GetCooldownTimeRemaining() > 0.0f;
}

void UUgWaitAbilityCooldown::Activate()
{
	Super::Activate();
	if (!ShouldBroadcastAbilityTaskDelegates()) { return; }
	if (bCheckForAlreadyCooledDown && Ability->IsActive() && !IsCoolingDown())
	{
		OnCooldownEnd.Broadcast();
		EndTask();
	}
}
