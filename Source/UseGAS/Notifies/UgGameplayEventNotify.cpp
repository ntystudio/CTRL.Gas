﻿// SPDX-FileCopyrightText: 2025 NTY.studio

#include "UseGAS/Notifies/UgGameplayEventNotify.h"

#include "AbilitySystemBlueprintLibrary.h"

#include "Components/SkeletalMeshComponent.h"

#include "Logging/MessageLog.h"

#include "Misc/UObjectToken.h"

#if WITH_EDITOR

void UUgGameplayEventNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, FAnimNotifyEventReference const& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	auto const Owner = MeshComp->GetOwner();
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventData.EventTag, EventData);
}

void UUgGameplayEventNotify::ValidateAssociatedAssets()
{
	static FName const NAME_AssetCheck("AssetCheck");

	if (!EventData.EventTag.IsValid())
	{
		FMessageLog AssetCheckLog(NAME_AssetCheck);

		auto const* ContainingAsset = GetContainingAsset();
		FText const MessageInvalidTag = FText::FromString(
			FString::Printf(TEXT("UgGameplayEventNotify: EventData.EventTag is not valid.")));
		AssetCheckLog.Warning()
			->AddToken(FUObjectToken::Create(ContainingAsset))
			->AddToken(FTextToken::Create(MessageInvalidTag));

		if (GIsEditor)
		{
			AssetCheckLog.Notify(MessageInvalidTag, EMessageSeverity::Warning, /*bForce=*/ true);
		}
	}
}

#endif