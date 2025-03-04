// Fill out your copyright notice in the Description page of Project Settings.

#include "CTRLGas/Targeting/CTRLWaitTaskEnd.h"

#include "GameplayTasksComponent.h"

UCTRLWaitTaskEnd::UCTRLWaitTaskEnd()
{
	bTickingTask = true;
}

UCTRLWaitTaskEnd* UCTRLWaitTaskEnd::WaitTaskEnd(UGameplayAbility* OwningAbility, FName TaskInstanceName, UGameplayTask* AsyncTask)
{
	auto* Task = NewAbilityTask<ThisClass>(OwningAbility, TaskInstanceName);
	if (!ensure(Task)) return nullptr;
	Task->AsyncTask = AsyncTask;
	return Task;
}

void UCTRLWaitTaskEnd::TickTask(float const DeltaTime)
{
	Super::TickTask(DeltaTime);
	if (IsDone())
	{
		// UE_LOG(LogTemp, Warning, TEXT("1 UCTRLWaitTaskEnd::TickTask: Task is done"));
		EndTask();
	}
}

void UCTRLWaitTaskEnd::Activate()
{
	// UE_LOG(LogTemp, Warning, TEXT("UCTRLWaitTaskEnd::Activate"));
	Super::Activate();
	if (IsDone())
	{
		// UE_LOG(LogTemp, Warning, TEXT("2 UCTRLWaitTaskEnd::TickTask: Task is done"));
		EndTask();
	}
}

void UCTRLWaitTaskEnd::OnGameplayTaskDeactivated(UGameplayTask& Task)
{
	// UE_LOG(LogTemp, Warning, TEXT("UCTRLWaitTaskEnd::OnGameplayTaskDeactivated"));
	if (IsDone())
	{
		// UE_LOG(LogTemp, Warning, TEXT("3 UCTRLWaitTaskEnd::TickTask: Task is done"));
		EndTask();
	}
	Super::OnGameplayTaskDeactivated(Task);
}

void UCTRLWaitTaskEnd::OnDestroy(bool const bInOwnerFinished)
{
	// UE_LOG(LogTemp, Warning, TEXT("OnDestroy"));
	if (IsDone())
	{
		// UE_LOG(LogTemp, Warning, TEXT("OnDestroy is done"));
		if (Ability)
		{
			// UE_LOG(LogTemp, Warning, TEXT("OnTaskEnd.Broadcast"));
			OnTaskEnd.Broadcast();
		}
		else
		{
			// UE_LOG(LogTemp, Warning, TEXT("not OnTaskEnd.Broadcast"));
		}
	}
	else
	{
		// UE_LOG(LogTemp, Warning, TEXT("OnDestroy not done"));
	}
	Super::OnDestroy(bInOwnerFinished);
}

bool UCTRLWaitTaskEnd::IsDone() const
{
	if (!AsyncTask.IsValid())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Is Done Invalid"));
		return true;
	}
	// UE_LOG(LogTemp, Warning, TEXT("Is Done %s %s"), *AsyncTask->GetDebugString(), *AsyncTask->GetTaskStateName());
	return AsyncTask->IsFinished();
}
