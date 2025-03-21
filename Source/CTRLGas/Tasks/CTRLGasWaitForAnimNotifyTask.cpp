﻿// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#include "CTRLGas/Tasks/CTRLGasWaitForAnimNotifyTask.h"

#include "AbilitySystemComponent.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

UAnimInstance* UCTRLGasWaitForAnimNotifyTask::GetAnimInstance() const
{
	return AnimInstance.Get();
}

TArray<FName> UCTRLGasWaitForAnimNotifyTask::GetNotifyNames() const
{
	TArray<FName> NotifyNames;
	if (ExpectedMontage)
	{
		NotifyNames.Reserve(ExpectedMontage->Notifies.Num());
		for (auto Notify : ExpectedMontage->Notifies)
		{
			NotifyNames.Add(Notify.NotifyName);
		}
	}
	return NotifyNames;
}

UCTRLGasWaitForAnimNotifyTask* UCTRLGasWaitForAnimNotifyTask::WaitForAnimNotify(UGameplayAbility* OwningAbility, FName const TaskInstanceName, UAnimMontage* InExpectedMontage, FName const InNotifyName)
{
	auto* Task = NewAbilityTask<UCTRLGasWaitForAnimNotifyTask>(OwningAbility, TaskInstanceName);
	Task->ExpectedMontage = InExpectedMontage;
	Task->ListenForNotify = InNotifyName;
	return Task;
}

void UCTRLGasWaitForAnimNotifyTask::Activate()
{
	Super::Activate();
	if (!ShouldBroadcastAbilityTaskDelegates()) { return; }

	auto const ASC = AbilitySystemComponent.Get();
	check(ASC);
	auto const ActorInfo = ASC->AbilityActorInfo;
	AnimInstance = ActorInfo->GetAnimInstance();
	if (!AnimInstance.IsValid())
	{
		return END_WITH_ERROR(Error, TEXT("CTRLGasWaitForAnimNotifyTask: No AnimInstance found"));
	}

	if (!IsValid(ExpectedMontage))
	{
		return END_WITH_ERROR(Error, TEXT("CTRLGasWaitForAnimNotifyTask: Expected Montage is not valid"));
	}
	if (!IsPlayingExpectedMontage())
	{
		return END_WITH_ERROR(Error, TEXT("CTRLGasWaitForAnimNotifyTask: Expected montage %s is not playing"), *GetNameSafe(ExpectedMontage));
	}

	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &ThisClass::OnNotifyBeginReceived);
	AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &ThisClass::OnNotifyEndReceived);
	AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::OnMontageEnded);
}

void UCTRLGasWaitForAnimNotifyTask::RemoveBindings() const
{
	if (!AnimInstance.IsValid()) { return; }
	AnimInstance->OnPlayMontageNotifyBegin.RemoveAll(this);
	AnimInstance->OnPlayMontageNotifyEnd.RemoveAll(this);
	AnimInstance->OnMontageEnded.RemoveAll(this);
}

void UCTRLGasWaitForAnimNotifyTask::BeginDestroy()
{
	RemoveBindings();
	Super::BeginDestroy();
}

void UCTRLGasWaitForAnimNotifyTask::OnDestroy(bool const AbilityEnded)
{
	RemoveBindings();
	Super::OnDestroy(AbilityEnded);
}

void UCTRLGasWaitForAnimNotifyTask::OnNotifyBeginReceived(FName const NotifyName, FBranchingPointNotifyPayload const& BranchingPointPayload)
{
	if (!ShouldBroadcastAbilityTaskDelegates()) { return; }
	if (!EnsureIsPlayingExpectedMontage()) return;
	if (NotifyName == ListenForNotify)
	{
		OnNotifyBegin.Broadcast();
	}
}

void UCTRLGasWaitForAnimNotifyTask::OnNotifyEndReceived(FName const NotifyName, FBranchingPointNotifyPayload const& BranchingPointPayload)
{
	if (!ShouldBroadcastAbilityTaskDelegates()) return;
	if (!EnsureIsPlayingExpectedMontage()) return;
	if (NotifyName == ListenForNotify)
	{
		OnNotifyEnd.Broadcast();
	}
}

bool UCTRLGasWaitForAnimNotifyTask::EnsureIsPlayingExpectedMontage()
{
	if (!ensure(IsPlayingExpectedMontage()))
	{
		END_WITH_ERROR(Error, TEXT("CTRLGasWaitForAnimNotifyTask: Expected montage %s is not playing."), *GetNameSafe(ExpectedMontage));
		return false;
	}
	return true;
}

bool UCTRLGasWaitForAnimNotifyTask::IsPlayingExpectedMontage() const
{
	if (!Ability) return false;
	auto const ASC = AbilitySystemComponent.Get();
	check(ASC);
	auto const ActorInfo = ASC->AbilityActorInfo;
	check(ActorInfo);
	if (ASC->GetAnimatingAbility() != Ability) return false;
	auto const CurrentAnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance.IsValid() && AnimInstance != CurrentAnimInstance) { return false; }
	return IsValid(CurrentAnimInstance) && CurrentAnimInstance->Montage_IsPlaying(ExpectedMontage);
}

void UCTRLGasWaitForAnimNotifyTask::OnMontageEnded(UAnimMontage* Montage, bool const bInterrupted)
{
	if (!ShouldBroadcastAbilityTaskDelegates()) return;
	if (Montage != ExpectedMontage)
	{
		return END_WITH_ERROR(Error, TEXT("CTRLGasWaitForAnimNotifyTask: Montage Ended %s is not the expected montage %s"), *GetNameSafe(Montage), *GetNameSafe(ExpectedMontage));
	}

	if (bInterrupted)
	{
		OnInterrupted.Broadcast();
		OnInterrupted.Clear();
	}

	if (!IsFinished())
	{
		EndTask();
	}
}
