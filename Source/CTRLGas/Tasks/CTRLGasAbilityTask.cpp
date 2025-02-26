// SPDX-FileCopyrightText: 2025 NTY.studio
#include "CTRLGasAbilityTask.h"

#include "CTRLGas/CTRLGasComponent.h"

void UCTRLGasAbilityTask::Activate()
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

void UCTRLGasAbilityTask::OnDestroy(bool const bInOwnerFinished)
{
	bool const bShouldBroadcast = !bInOwnerFinished || ShouldBroadcastAbilityTaskDelegates();
	Super::OnDestroy(bInOwnerFinished);
	if (bShouldBroadcast)
	{
		OnCompleted.Broadcast();
		OnCompleted.Clear();
	}
}

UCTRLGasComponent* UCTRLGasAbilityTask::GetASC() const
{
	return Cast<UCTRLGasComponent>(AbilitySystemComponent.Get());
}
