// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"

#include "Abilities/GameplayAbilityTargetActor_SingleLineTrace.h"

#include "CTRLCore/CTRLCollision.h"

#include "CTRLTargetActor_SingleLineTrace.generated.h"

UCLASS(Blueprintable)
class CTRLGAS_API ACTRLTargetActor_SingleLineTrace : public AGameplayAbilityTargetActor_SingleLineTrace
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn))
	FCTRLCollisionQueryParams CollisionParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ExposeOnSpawn, EditCondition="bDebug"))
	FCTRLDrawDebugOptions DrawDebugOptions;

protected:
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;
};
