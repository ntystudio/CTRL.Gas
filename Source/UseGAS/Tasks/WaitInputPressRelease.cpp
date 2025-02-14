﻿// SPDX-FileCopyrightText: 2025 NTY.studio

#include "WaitInputPressRelease.h"

#include "Engine/World.h"

#include "UseGAS/UseGASAbilitySystemComponent.h"

void UWaitInputPressRelease::OnPressCallback()
{
	float const ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;

	auto* ASC = GetASC();
	if (!Ability || !ASC) return;

	auto const AbilityHandle = GetAbilitySpecHandle();
	auto const AbilityOriginalPredictionKey = GetActivationPredictionKey();
	if (bEndOnPressed)
	{
		ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, AbilityHandle, AbilityOriginalPredictionKey).Remove(PressedDelegateHandle);
	}

	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	if (IsPredictingClient())
	{
		// Tell the server about this
		ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilityHandle, AbilityOriginalPredictionKey, ASC->ScopedPredictionKey);
	}
	else
	{
		ASC->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilityHandle, AbilityOriginalPredictionKey);
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnPress.Broadcast(ElapsedTime);
	}

	if (bEndOnPressed)
	{
		// We are done. Kill us so we don't keep getting broadcast messages
		EndTask();
	}
}

void UWaitInputPressRelease::OnReleaseCallback()
{
	float const ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;

	auto* ASC = GetASC();
	if (!Ability || !ASC) return;

	auto const AbilityHandle = GetAbilitySpecHandle();
	auto const AbilityOriginalPredictionKey = GetActivationPredictionKey();
	if (bEndOnReleased)
	{
		ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputReleased, AbilityHandle, AbilityOriginalPredictionKey).Remove(ReleasedDelegateHandle);
	}

	FScopedPredictionWindow ScopedPrediction(ASC, IsPredictingClient());

	if (IsPredictingClient())
	{
		// Tell the server about this
		ASC->ServerSetReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilityHandle, AbilityOriginalPredictionKey, ASC->ScopedPredictionKey);
	}
	else
	{
		ASC->ConsumeGenericReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilityHandle, AbilityOriginalPredictionKey);
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnRelease.Broadcast(ElapsedTime);
	}

	if (bEndOnReleased)
	{
		// We are done. Kill us so we don't keep getting broadcast messages
		EndTask();
	}
}

UWaitInputPressRelease* UWaitInputPressRelease::WaitInputPressRelease(
	UGameplayAbility* OwningAbility,
	bool const bInTestAlreadyPressed,
	bool const bInTestAlreadyReleased,
	bool const bInEndOnPressed,
	bool const bInEndOnReleased
)
{
	UWaitInputPressRelease* Task = NewAbilityTask<UWaitInputPressRelease>(OwningAbility);
	if (!Task) return nullptr;
	Task->bTestAlreadyPressed = bInTestAlreadyPressed;
	Task->bTestAlreadyReleased = bInTestAlreadyReleased;
	Task->bEndOnPressed = bInEndOnPressed;
	Task->bEndOnReleased = bInEndOnReleased;
	return Task;
}

void UWaitInputPressRelease::Activate()
{
	StartTime = GetWorld()->GetTimeSeconds();
	auto* ASC = GetASC();
	if (!ASC) return END_WITH_ERROR(Log, TEXT("UWaitInputPressRelease: Invalid ASC."));
	if (!Ability) return END_WITH_ERROR(Log, TEXT("UWaitInputPressRelease: Invalid Ability."));

	if (IsLocallyControlled())
	{
		if (auto const* Spec = Ability->GetCurrentAbilitySpec())
		{
			if (bTestAlreadyPressed && Spec->InputPressed)
			{
				OnPressCallback();
				if (bEndOnPressed) return;
			}
			if (bTestAlreadyReleased && !Spec->InputPressed)
			{
				OnReleaseCallback();
				if (bEndOnReleased) return;
			}
		}
	}

	auto const AbilityHandle = GetAbilitySpecHandle();
	auto const AbilityOriginalPredictionKey = GetActivationPredictionKey();
	PressedDelegateHandle = ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputPressed, AbilityHandle, AbilityOriginalPredictionKey)
		.AddUObject(this, &UWaitInputPressRelease::OnPressCallback);
	ReleasedDelegateHandle = ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::InputReleased, AbilityHandle, AbilityOriginalPredictionKey)
		.AddUObject(this, &UWaitInputPressRelease::OnReleaseCallback);

	if (IsForRemoteClient())
	{
		if (
			!ASC->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputPressed, AbilityHandle, AbilityOriginalPredictionKey) ||
			!ASC->CallReplicatedEventDelegateIfSet(EAbilityGenericReplicatedEvent::InputReleased, AbilityHandle, AbilityOriginalPredictionKey)
		)
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}
