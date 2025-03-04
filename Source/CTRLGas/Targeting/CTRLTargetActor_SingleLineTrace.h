// Fill out your copyright notice in the Description page of Project Settings.

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
