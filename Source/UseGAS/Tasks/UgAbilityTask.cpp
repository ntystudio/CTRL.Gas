// SPDX-FileCopyrightText: 2025 NTY.studio
#include "UgAbilityTask.h"

void UUgAbilityTask::Activate()
{
	Super::Activate();
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnActivated.Broadcast();
		OnActivated.Clear();
	}
	else if (IsValid(Ability))
	{
		OnError.Broadcast();
		OnError.Clear();
	}
}

void UUgAbilityTask::OnDestroy(bool const bInOwnerFinished)
{
	bool const bShouldBroadcast = !bInOwnerFinished || ShouldBroadcastAbilityTaskDelegates();
	Super::OnDestroy(bInOwnerFinished);
	if (bShouldBroadcast)
	{
		OnCompleted.Broadcast();
		OnCompleted.Clear();
	}
}