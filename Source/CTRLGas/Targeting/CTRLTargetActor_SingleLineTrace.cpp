// Fill out your copyright notice in the Description page of Project Settings.

#include "CTRLGas/Targeting/CTRLTargetActor_SingleLineTrace.h"

#include "DrawDebugHelpers.h"
#include "KismetTraceUtils.h"

// Sets default values
FHitResult ACTRLTargetActor_SingleLineTrace::PerformTrace(AActor* InSourceActor)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(InSourceActor);

	FCollisionQueryParams Params = CollisionParams.ToCollisionQueryParams();
	Params.TraceTag = SCENE_QUERY_STAT(AGameplayAbilityTargetActor_SingleLineTrace);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);

	FVector const TraceStart = StartLocation.GetTargetingTransform().GetLocation(); // InSourceActor->GetActorLocation();
	FVector TraceEnd;
	AimWithPlayerController(InSourceActor, Params, TraceStart, TraceEnd); //Effective on server and launching client only

	// ------------------------------------------------------

	FHitResult ReturnHitResult;
	LineTraceWithFilter(ReturnHitResult, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);
	//Default to end of trace line if we don't hit anything.
	FColor Color = FColor::Green;
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
	}
	else
	{
		Color = FColor::Yellow;
	}

	if (auto* LocalReticleActor = ReticleActor.Get())
	{
		bool const bHitActor = (ReturnHitResult.bBlockingHit && (ReturnHitResult.HitObjectHandle.IsValid()));
		FVector const ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor)
			? ReturnHitResult.HitObjectHandle.GetLocation()
			: ReturnHitResult.Location;

		LocalReticleActor->SetActorLocation(ReticleLocation);
		LocalReticleActor->SetIsTargetAnActor(bHitActor);
		if (bHitActor) Color = FColor::Red;
	}

#if ENABLE_DRAW_DEBUG
	if (DrawDebugOptions.bDrawDebug)
	{
		DrawDebugLineTraceSingle(
			GetWorld(),
			TraceStart,
			TraceEnd,
			DrawDebugOptions.GetDrawDebugType(),
			ReturnHitResult.bBlockingHit,
			ReturnHitResult,
			Color,
			FColor::Red,
			DrawDebugOptions.LifeTime
		);
		// DrawDebugSphere(GetWorld(), AimEnd, 25.0f, 16, Color);
	}
#endif
	return MoveTemp(ReturnHitResult);
}
