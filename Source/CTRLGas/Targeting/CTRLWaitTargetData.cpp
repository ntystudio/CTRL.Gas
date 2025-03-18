// Fill out your copyright notice in the Description page of Project Settings.

#include "CTRLGas/Targeting/CTRLWaitTargetData.h"

#include "AbilitySystemComponent.h"

#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"

#include "Engine/Engine.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CTRLWaitTargetData)

UCTRLWaitTargetDataTask::UCTRLWaitTargetDataTask(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer) {}

UCTRLWaitTargetDataTask* UCTRLWaitTargetDataTask::WaitTargetDataUsingReusableActor(
	UGameplayAbility* OwningAbility,
	FName const TaskInstanceName,
	TEnumAsByte<EGameplayTargetingConfirmation::Type> const InConfirmationType,
	AGameplayAbilityTargetActor* InTargetActor,
	bool const bInDestroyTargetActor
)
{
	UCTRLWaitTargetDataTask* Task = NewAbilityTask<UCTRLWaitTargetDataTask>(OwningAbility, TaskInstanceName); //Register for task list here, providing a given FName as a key
	if (!Task) return nullptr;
	Task->TargetActor = InTargetActor;
	Task->ConfirmationType = InConfirmationType;
	Task->bDestroyTargetActor = bInDestroyTargetActor;
	return Task;
}

void UCTRLWaitTargetDataTask::Activate()
{
	// Need to handle case where target actor was passed into task
	if (!Ability) return;
	if (TargetActor)
	{
		RegisterTargetDataCallbacks();

		if (!IsValid(this))
		{
			return;
		}

		InitializeTargetActor(TargetActor);
		FinalizeTargetActor(TargetActor);

		// Note that the call to FinalizeTargetActor, this task could finish and our owning ability may be ended.
	}
	else
	{
		EndTask();
	}
}

// bool UCTRLWaitTargetDataTask::BeginSpawningActor(UGameplayAbility* OwningAbility, TSubclassOf<AGameplayAbilityTargetActor> const InTargetClass, AGameplayAbilityTargetActor*& SpawnedActor)
// {
// 	SpawnedActor = TargetActor;
//
// 	if (Ability)
// 	{
// 		if (ShouldSpawnTargetActor())
// 		{
// 			UClass* Class = *InTargetClass;
// 			if (Class != nullptr && !SpawnedActor)
// 			{
// 				if (UWorld* World = GEngine->GetWorldFromContextObject(OwningAbility, EGetWorldErrorMode::LogAndReturnNull))
// 				{
// 					SpawnedActor = World->SpawnActorDeferred<AGameplayAbilityTargetActor>(Class, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
// 				}
// 			}
//
// 			// if (SpawnedActor)
// 			// {
// 			// 	// TargetActor = SpawnedActor;
// 			// 	// TargetClass = SpawnedActor->GetClass();
// 			// 	InitializeTargetActor(SpawnedActor);
// 			// }
// 		}
//
// 		// RegisterTargetDataCallbacks();
// 	}
//
// 	return (SpawnedActor != nullptr);
// }

// void UCTRLWaitTargetDataTask::FinishSpawningActor(UGameplayAbility* OwningAbility, AGameplayAbilityTargetActor* SpawnedActor)
// {
// 	if (SpawnedActor)
// 	{
// 		// TargetActor = SpawnedActor;
// 		// TargetClass = SpawnedActor->GetClass();
// 	}
// 	InitializeTargetActor(SpawnedActor);
// 	RegisterTargetDataCallbacks();
//
// 	UAbilitySystemComponent const* ASC = AbilitySystemComponent.Get();
// 	if (ASC && IsValid(SpawnedActor))
// 	{
// 		// check(TargetActor == SpawnedActor);
//
// 		FTransform const SpawnTransform = ASC->GetOwner()->GetTransform();
//
// 		if (!SpawnedActor->IsActorInitialized())
// 		{
// 			SpawnedActor->FinishSpawning(SpawnTransform);
// 			// Broadcast that we created a target actor
// 			TargetActorCreated.Broadcast(SpawnedActor);
// 		}
//
// 		FinalizeTargetActor(SpawnedActor);
// 	}
// }

// bool UCTRLWaitTargetDataTask::ShouldSpawnTargetActor() const
// {
// 	check(TargetClass);
// 	check(Ability);
//
// 	// Spawn the actor if this is a locally controlled ability (always) or if this is a replicating targeting mode.
// 	// (E.g., server will spawn this target actor to replicate to all non owning clients)
//
// 	AGameplayAbilityTargetActor const* CDO = CastChecked<AGameplayAbilityTargetActor>(TargetClass->GetDefaultObject());
//
// 	bool const bReplicates = CDO->GetIsReplicated();
// 	bool const bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
// 	bool const bShouldProduceTargetDataOnServer = CDO->ShouldProduceTargetDataOnServer;
//
// 	return (bReplicates || bIsLocallyControlled || bShouldProduceTargetDataOnServer);
// }

void UCTRLWaitTargetDataTask::InitializeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const
{
	check(SpawnedActor);
	check(Ability);

	SpawnedActor->PrimaryPC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	SpawnedActor->bDestroyOnConfirmation = bDestroyTargetActor;

	// If we spawned the target actor, always register the callbacks for when the data is ready.
	SpawnedActor->TargetDataReadyDelegate.RemoveAll(this);
	SpawnedActor->TargetDataReadyDelegate.AddUObject(const_cast<UCTRLWaitTargetDataTask*>(this), &UCTRLWaitTargetDataTask::OnTargetDataReadyCallback);
	SpawnedActor->CanceledDelegate.RemoveAll(this);
	SpawnedActor->CanceledDelegate.AddUObject(const_cast<UCTRLWaitTargetDataTask*>(this), &UCTRLWaitTargetDataTask::OnTargetDataCancelledCallback);
}

void UCTRLWaitTargetDataTask::FinalizeTargetActor(AGameplayAbilityTargetActor* SpawnedActor) const
{
	check(SpawnedActor);
	check(Ability);

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		// User ability activation is inhibited while this is active
		ASC->SpawnedTargetActors.Push(SpawnedActor);
	}

	SpawnedActor->StartTargeting(Ability);

	if (SpawnedActor->ShouldProduceTargetData())
	{
		// If instant confirm, then stop targeting immediately.
		// Note this is kind of bad: we should be able to just call a static func on the CDO to do this. 
		// But then we wouldn't get to set ExposeOnSpawnParameters.
		if (ConfirmationType == EGameplayTargetingConfirmation::Instant)
		{
			SpawnedActor->ConfirmTargeting();
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			// Bind to the Cancel/Confirm Delegates (called from local confirm or from repped confirm)
			SpawnedActor->BindToConfirmCancelInputs();
		}
	}
}

void UCTRLWaitTargetDataTask::RegisterTargetDataCallbacks()
{
	if (!ensure(IsValid(this))) return;

	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC) return;

	check(TargetActor);
	check(Ability);

	AGameplayAbilityTargetActor const* CDO = CastChecked<AGameplayAbilityTargetActor>(TargetActor->GetClass()->GetDefaultObject());

	bool const bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	bool const bShouldProduceTargetDataOnServer = CDO->ShouldProduceTargetDataOnServer;

	// If not locally controlled (server for remote client), see if TargetData was already sent
	// else register callback for when it does get here.
	if (!bIsLocallyControlled)
	{
		// Register with the TargetData callbacks if we are expecting client to send them
		if (!bShouldProduceTargetDataOnServer)
		{
			FGameplayAbilitySpecHandle const SpecHandle = GetAbilitySpecHandle();
			FPredictionKey const ActivationPredictionKey = GetActivationPredictionKey();

			//Since multifire is supported, we still need to hook up the callbacks
			ASC->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UCTRLWaitTargetDataTask::OnTargetDataReplicatedCallback);
			ASC->AbilityTargetDataCancelledDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UCTRLWaitTargetDataTask::OnTargetDataReplicatedCancelledCallback);

			ASC->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);

			SetWaitingOnRemotePlayerData();
		}
	}
}

/** Valid TargetData was replicated to use (we are server, was sent from client) */
void UCTRLWaitTargetDataTask::OnTargetDataReplicatedCallback(FGameplayAbilityTargetDataHandle const& Data, FGameplayTag ActivationTag)
{
	FGameplayAbilityTargetDataHandle MutableData = Data;

	if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
	{
		ASC->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	}

	/** 
	 *  Call into the TargetActor to sanitize/verify the data. If this returns false, we are rejecting
	 *	the replicated target data and will treat this as a cancel.
	 *	
	 *	This can also be used for bandwidth optimizations. OnReplicatedTargetDataReceived could do an actual
	 *	trace/check/whatever server side and use that data. So rather than having the client send that data
	 *	explicitly, the client is basically just sending a 'confirm' and the server is now going to do the work
	 *	in OnReplicatedTargetDataReceived.
	 */
	if (TargetActor && !TargetActor->OnReplicatedTargetDataReceived(MutableData))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			Cancelled.Broadcast(MutableData);
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			ValidData.Broadcast(MutableData);
		}
	}

	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

/** Client canceled this Targeting Task (we are the server) */
void UCTRLWaitTargetDataTask::OnTargetDataReplicatedCancelledCallback()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	EndTask();
}

/** The TargetActor we spawned locally has called back with valid target data */
void UCTRLWaitTargetDataTask::OnTargetDataReadyCallback(FGameplayAbilityTargetDataHandle const& Data)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!Ability || !ASC)
	{
		return;
	}

	FScopedPredictionWindow ScopedPrediction(ASC, ShouldReplicateDataToServer());

	FGameplayAbilityActorInfo const* Info = Ability->GetCurrentActorInfo();
	if (IsPredictingClient())
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			FGameplayTag const ApplicationTag; // Fixme: where would this be useful?
			ASC->CallServerSetReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey(), Data, ApplicationTag, ASC->ScopedPredictionKey);
		}
		else if (ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed)
		{
			// We aren't going to send the target data, but we will send a generic confirmed message.
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(Data);
	}

	if (ConfirmationType != EGameplayTargetingConfirmation::CustomMulti)
	{
		EndTask();
	}
}

/** The TargetActor we spawned locally has called back with a cancel event (they still include the 'last/best' targetdata but the consumer of this may want to discard it) */
void UCTRLWaitTargetDataTask::OnTargetDataCancelledCallback(FGameplayAbilityTargetDataHandle const& Data)
{
	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
	if (!ASC)
	{
		return;
	}

	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	if (IsPredictingClient())
	{
		if (!TargetActor->ShouldProduceTargetDataOnServer)
		{
			ASC->ServerSetReplicatedTargetDataCancelled(GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
		else
		{
			// We aren't going to send the target data, but we will send a generic confirmed message.
			ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), ASC->ScopedPredictionKey);
		}
	}
	Cancelled.Broadcast(Data);
	EndTask();
}

/** Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true. */
void UCTRLWaitTargetDataTask::ExternalConfirm(bool const bEndTask)
{
	if (TargetActor)
	{
		if (TargetActor->ShouldProduceTargetData())
		{
			TargetActor->ConfirmTargetingAndContinue();
		}
	}
	Super::ExternalConfirm(bEndTask);
}

/** Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true. */
void UCTRLWaitTargetDataTask::ExternalCancel()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		Cancelled.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	Super::ExternalCancel();
}

void UCTRLWaitTargetDataTask::OnDestroy(bool const AbilityEnded)
{
	if (TargetActor && bDestroyTargetActor)
	{
		TargetActor->Destroy();
	}

	Super::OnDestroy(AbilityEnded);
}

bool UCTRLWaitTargetDataTask::ShouldReplicateDataToServer() const
{
	if (!Ability || !TargetActor)
	{
		return false;
	}

	// Send TargetData to the server IFF we are the client and this isn't a GameplayTargetActor that can produce data on the server	
	FGameplayAbilityActorInfo const* Info = Ability->GetCurrentActorInfo();
	if (!Info->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer)
	{
		return true;
	}

	return false;
}

// --------------------------------------------------------------------------------------
