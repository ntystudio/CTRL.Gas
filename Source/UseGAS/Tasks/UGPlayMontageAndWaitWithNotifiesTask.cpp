// // SPDX-FileCopyrightText: 2025 NTY.studio
//
// #include "UseGAS/Tasks/UGPlayMontageAndWaitWithNotifiesTask.h"
// // SPDX-FileCopyrightText: 2025 NTY.studio
//
// #include "AbilitySystemComponent.h"
// #include "AbilitySystemGlobals.h"
// #include "AbilitySystemLog.h"
//
// #include "Animation/AnimMontage.h"
//
// #include "GameFramework/Character.h"
//
// #include UE_INLINE_GENERATED_CPP_BY_NAME(UGPlayMontageAndWaitWithNotifiesTask)
//
// static bool GUseAggressivePlayMontageAndWaitEndTask = true;
// static FAutoConsoleVariableRef CVarAggressivePlayMontageAndWaitEndTask(
// 	TEXT("AbilitySystem.PlayMontage.AggressiveEndTask"),
// 	GUseAggressivePlayMontageAndWaitEndTask,
// 	TEXT("This should be set to true in order to avoid multiple callbacks off an AbilityTask_PlayMontageAndWait node")
// );
//
// static bool GPlayMontageAndWaitFireInterruptOnAnimEndInterrupt = true;
// static FAutoConsoleVariableRef CVarPlayMontageAndWaitFireInterruptOnAnimEndInterrupt(
// 	TEXT("AbilitySystem.PlayMontage.FireInterruptOnAnimEndInterrupt"),
// 	GPlayMontageAndWaitFireInterruptOnAnimEndInterrupt,
// 	TEXT("This is a fix that will cause AbilityTask_PlayMontageAndWait to fire its Interrupt event if the underlying AnimInstance ends in an interrupted")
// );
//
//
// bool UUGPlayMontageAndWaitWithNotifiesTask::IsPlayingThisMontage() const
// {
// 	if (!Ability) return false;
// 	if (!AnimInstance) return false;
// 	if (auto ASC = AbilitySystemComponent.Get())
// 	{
// 		if (ASC->GetAnimatingAbility() != Ability) return false;
// 		if (auto const ActorInfo = Ability->GetCurrentActorInfo())
// 		{
// 			if (auto const CurrentAnimInstance = ActorInfo->GetAnimInstance())
// 			{
// 				return CurrentAnimInstance == AnimInstance && CurrentAnimInstance->Montage_IsPlaying(MontageToPlay);
// 			}
// 		}
// 	}
// 	return false;
// }
// void UUGPlayMontageAndWaitWithNotifiesTask::OnMontageBlendingOut(UAnimMontage* Montage, bool const bInterrupted)
// {
// 	bool const bPlayingThisMontage = IsPlayingThisMontage();
// 	if (bPlayingThisMontage)
// 	{
// 		// Reset AnimRootMotionTranslationScale
// 		if (auto const Character = Cast<ACharacter>(GetAvatarActor()))
// 		{
// 			if ((Character->GetLocalRole() == ROLE_Authority ||
// 				(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
// 			{
// 				Character->SetAnimRootMotionTranslationScale(1.f);
// 			}
// 		}
// 	}
//
// 	if (bPlayingThisMontage && (bInterrupted || !bAllowInterruptAfterBlendOut))
// 	{
// 		if (UAbilitySystemComponent* ASC = AbilitySystemComponent.Get())
// 		{
// 			ASC->ClearAnimatingAbility(Ability);
// 		}
// 	}
//
// 	if (ShouldBroadcastAbilityTaskDelegates())
// 	{
// 		if (bInterrupted)
// 		{
// 			bAllowInterruptAfterBlendOut = false;
// 			OnInterrupted.Broadcast();
//
// 			if (GUseAggressivePlayMontageAndWaitEndTask)
// 			{
// 				EndTask();
// 			}
// 		}
// 		else
// 		{
// 			OnBlendOut.Broadcast();
// 		}
// 	}
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::OnMontageBlendedIn(UAnimMontage* Montage)
// {
// 	if (ShouldBroadcastAbilityTaskDelegates())
// 	{
// 		OnBlendedIn.Broadcast();
// 	}
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::OnMontageInterrupted()
// {
// 	// Call the new function
// 	OnGameplayAbilityCancelled();
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::OnGameplayAbilityCancelled()
// {
// 	if (StopPlayingMontage() || bAllowInterruptAfterBlendOut)
// 	{
// 		// Let the BP handle the interrupt as well
// 		if (ShouldBroadcastAbilityTaskDelegates())
// 		{
// 			bAllowInterruptAfterBlendOut = false;
// 			OnInterrupted.Broadcast();
// 		}
// 	}
//
// 	if (GUseAggressivePlayMontageAndWaitEndTask)
// 	{
// 		EndTask();
// 	}
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::OnMontageEnded(UAnimMontage* Montage, bool const bInterrupted)
// {
// 	if (!bInterrupted)
// 	{
// 		if (ShouldBroadcastAbilityTaskDelegates())
// 		{
// 			OnCompleted.Broadcast();
// 		}
// 	}
// 	else if (bAllowInterruptAfterBlendOut && GPlayMontageAndWaitFireInterruptOnAnimEndInterrupt)
// 	{
// 		if (ShouldBroadcastAbilityTaskDelegates())
// 		{
// 			OnInterrupted.Broadcast();
// 		}
// 	}
//
// 	EndTask();
// }
//
// UUGPlayMontageAndWaitWithNotifiesTask* UUGPlayMontageAndWaitWithNotifiesTask::CreatePlayMontageAndWaitWithNotifiesProxy(
// 	UGameplayAbility* OwningAbility,
// 	FName const TaskInstanceName,
// 	UAnimMontage* MontageToPlay,
// 	float Rate,
// 	FName const StartSection,
// 	bool const bStopWhenAbilityEnds,
// 	float const AnimRootMotionTranslationScale,
// 	float const StartTimeSeconds,
// 	bool const bAllowInterruptAfterBlendOut
// )
// {
// 	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);
//
// 	UUGPlayMontageAndWaitWithNotifiesTask* MyObj = NewAbilityTask<UUGPlayMontageAndWaitWithNotifiesTask>(OwningAbility, TaskInstanceName);
// 	MyObj->MontageToPlay = MontageToPlay;
// 	MyObj->Rate = Rate;
// 	MyObj->StartSection = StartSection;
// 	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
// 	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
// 	MyObj->bAllowInterruptAfterBlendOut = bAllowInterruptAfterBlendOut;
// 	MyObj->StartTimeSeconds = StartTimeSeconds;
//
// 	return MyObj;
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::Activate()
// {
// 	if (Ability == nullptr) return;
//
// 	UAbilitySystemComponent* ASC = AbilitySystemComponent.Get();
// 	if (!ASC)
// 	{
// 		ABILITY_LOG(Warning, TEXT("UUGPlayMontageAndWaitWithNotifiesTask called on invalid AbilitySystemComponent"));
// 		return;
// 	}
// 	auto const ActorInfo = Ability->GetCurrentActorInfo();
// 	AnimInstance = ActorInfo->GetAnimInstance();
// 	if (!AnimInstance)
// 	{
// 		ABILITY_LOG(Warning, TEXT("UUGPlayMontageAndWaitWithNotifiesTask call to PlayMontage failed!"));
// 		return;
// 	}
// 	MontageLength = ASC->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection, StartTimeSeconds);
// 	if (MontageLength > 0.f) // if played montage
// 	{
// 		// Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are pending kill.
// 		if (!ShouldBroadcastAbilityTaskDelegates()) { return; }
// 		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UUGPlayMontageAndWaitWithNotifiesTask::OnNotifyBeginReceived);
// 		AnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UUGPlayMontageAndWaitWithNotifiesTask::OnNotifyEndReceived);
//
// 		InterruptedHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UUGPlayMontageAndWaitWithNotifiesTask::OnGameplayAbilityCancelled);
//
// 		BlendedInDelegate.BindUObject(this, &UUGPlayMontageAndWaitWithNotifiesTask::OnMontageBlendedIn);
// 		AnimInstance->Montage_SetBlendedInDelegate(BlendedInDelegate, MontageToPlay);
//
// 		BlendingOutDelegate.BindUObject(this, &UUGPlayMontageAndWaitWithNotifiesTask::OnMontageBlendingOut);
// 		AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);
//
// 		MontageEndedDelegate.BindUObject(this, &UUGPlayMontageAndWaitWithNotifiesTask::OnMontageEnded);
// 		AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);
//
// 		if (auto const Character = Cast<ACharacter>(GetAvatarActor()))
// 		{
// 			if ((Character->GetLocalRole() == ROLE_Authority ||
// 				(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
// 			{
// 				Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
// 			}
// 		}
// 	} else {
// 		ABILITY_LOG(
// 			Warning,
// 			TEXT("UUGPlayMontageAndWaitWithNotifiesTask called in Ability %s failed to play montage %s; Task Instance Name %s."),
// 			*Ability->GetName(),
// 			*GetNameSafe(MontageToPlay),
// 			*InstanceName.ToString()
// 		);
// 		if (ShouldBroadcastAbilityTaskDelegates())
// 		{
// 			OnCancelled.Broadcast();
// 		}
// 	}
//
// 	SetWaitingOnAvatar();
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::ExternalCancel()
// {
// 	if (ShouldBroadcastAbilityTaskDelegates())
// 	{
// 		OnCancelled.Broadcast();
// 	}
// 	Super::ExternalCancel();
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::OnDestroy(bool const AbilityEnded)
// {
// 	// Note: Clearing montage end delegate isn't necessary since its not a multicast and will be cleared when the next montage plays.
// 	// (If we are destroyed, it will detect this and not do anything)
//
// 	// This delegate, however, should be cleared as it is a multicast
// 	if (Ability)
// 	{
// 		Ability->OnGameplayAbilityCancelled.Remove(InterruptedHandle);
// 		if (AbilityEnded && bStopWhenAbilityEnds)
// 		{
// 			StopPlayingMontage();
// 		}
// 	}
//
// 	ClearDelegates();
//
// 	Super::OnDestroy(AbilityEnded);
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::ClearDelegates()
// {
// 	if (!AnimInstance) return;
// 	// Unbind delegates so they don't get called as well
// 	AnimInstance->OnPlayMontageNotifyBegin.RemoveAll(this);
// 	AnimInstance->OnPlayMontageNotifyEnd.RemoveAll(this);
// 	if (auto const MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay))
// 	{
// 		MontageInstance->OnMontageBlendedInEnded.Unbind();
// 		MontageInstance->OnMontageBlendingOutStarted.Unbind();
// 		MontageInstance->OnMontageEnded.Unbind();
// 	}
// 	AnimInstance = nullptr;
// }
//
// bool UUGPlayMontageAndWaitWithNotifiesTask::StopPlayingMontage()
// {
// 	bool const bPlayingThisMontage = IsPlayingThisMontage();
// 	ClearDelegates();
// 	if (!bPlayingThisMontage) return false;
// 	// Check if the montage is still playing
// 	// The ability would have been interrupted, in which case we should automatically stop the montage
// 	if (auto const ASC = AbilitySystemComponent.Get())
// 	{
// 		ASC->CurrentMontageStop();
// 		return true;
// 	}
//
// 	return false;
// }
//
//
// FString UUGPlayMontageAndWaitWithNotifiesTask::GetDebugString() const
// {
// 	UAnimMontage const* PlayingMontage = nullptr;
// 	if (Ability)
// 	{
// 		auto const ActorInfo = Ability->GetCurrentActorInfo();
//
// 		if (auto const CurrentAnimInstance = ActorInfo->GetAnimInstance())
// 		{
// 			PlayingMontage = CurrentAnimInstance->Montage_IsActive(MontageToPlay) ? ToRawPtr(MontageToPlay) : CurrentAnimInstance->GetCurrentActiveMontage();
// 		}
// 	}
//
// 	return FString::Printf(TEXT("PlayMontageAndWait. MontageToPlay: %s  (Currently Playing): %s"), *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::OnNotifyBeginReceived(FName const NotifyName, FBranchingPointNotifyPayload const& BranchingPointPayload)
// {
// 	if (!ShouldBroadcastAbilityTaskDelegates()) { return; }
// 	if (OnNotifyBegin.IsBound())
// 	{
// 		OnNotifyBegin.Broadcast(NotifyName);
// 	}
// }
//
// void UUGPlayMontageAndWaitWithNotifiesTask::OnNotifyEndReceived(FName const NotifyName, FBranchingPointNotifyPayload const& BranchingPointPayload)
// {
// 	if (!ShouldBroadcastAbilityTaskDelegates()) { return; }
// 	if (OnNotifyEnd.IsBound())
// 	{
// 		OnNotifyEnd.Broadcast(NotifyName);
// 	}
// }
//
