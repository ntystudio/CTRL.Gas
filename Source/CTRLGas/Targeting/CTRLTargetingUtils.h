// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "CTRLTargetingUtils.generated.h"



UENUM()
enum class ECTRLTargetDataValid: uint8
{
	Valid,
	NotValid,
};
struct FGameplayAbilityTargetDataHandle;
/**
 * 
 */
UCLASS(DisplayName="Targeting Utils [CTRL]", Category="CTRL|Gas|Targeting")
class CTRLGAS_API UCTRLTargetingUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(
		BlueprintCallable,
		DisplayName = "Get Actor From Target Data [CTRL]",
		Category="CTRL|Gas|Targeting",
		meta = (
			ActorClass = "/Script/Engine.Actor",
			DeterminesOutputType = "ActorClass",
			ExpandEnumAsExecs = "ActorIsValid"
		)
	)
	static AActor* GetActorFromTargetData(FGameplayAbilityTargetDataHandle const& TargetData, TSubclassOf<AActor> ActorClass, UPARAM(DisplayName="IsFound") ECTRLTargetDataValid& ActorIsValid, int32 DataIndex = 0, int32 ActorIndex = 0);
};
