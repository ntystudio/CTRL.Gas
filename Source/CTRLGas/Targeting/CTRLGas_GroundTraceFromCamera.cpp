// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#include "CTRLGas_GroundTraceFromCamera.h"

#include "Abilities/GameplayAbility.h"

#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"

#include "GameFramework/PlayerController.h"

#include "Kismet/GameplayStatics.h"

FHitResult ACTRLGas_GroundTraceFromCamera::PerformTrace(AActor* InSourceActor)
{
	constexpr bool bTraceComplex = false;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ACTRLGas_GroundTraceFromCamera), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(InSourceActor);

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	APlayerController const* PC = OwningAbility->GetCurrentActorInfo()->PlayerController.Get();
	FVector TraceEnd;
	if (auto const* LocalPlayer = PC->GetLocalPlayer())
	{
		if (auto const ViewportClient = LocalPlayer->ViewportClient)
		{
			FVector2D MousePosition;
			if (ViewportClient->GetMousePosition(MousePosition))
			{
				FVector WorldOrigin;
				FVector WorldDirection;
				if (UGameplayStatics::DeprojectScreenToWorld(PC, MousePosition, WorldOrigin, WorldDirection))
				{
					TraceEnd = TraceStart + WorldDirection * MaxRange;
				}
			}
		}
	}

	// ------------------------------------------------------

	FHitResult ReturnHitResult;
	//Use a line trace initially to see where the player is actually pointing
	LineTraceWithFilter(ReturnHitResult, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);
	//Default to end of trace line if we don't hit anything.
	if (!ReturnHitResult.bBlockingHit)
	{
		ReturnHitResult.Location = TraceEnd;
	}

	//Second trace, straight down. Consider using InSourceActor->GetWorld()->NavigationSystem->ProjectPointToNavigation() instead of just going straight down in the case of movement abilities (flag/bool).
	TraceStart = ReturnHitResult.Location - (TraceEnd - TraceStart).GetSafeNormal(); //Pull back very slightly to avoid scraping down walls
	TraceEnd = TraceStart;
	TraceStart.Z += CollisionHeightOffset;
	TraceEnd.Z -= 99999.0f;
	ReturnHitResult.Reset(1.0f);
	LineTraceWithFilter(ReturnHitResult, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);
	//if (!ReturnHitResult.bBlockingHit) then our endpoint may be off the map. Hopefully this is only possible in debug maps.

	bLastTraceWasGood = true; //So far, we're good. If we need a ground spot and can't find one, we'll come back.

	//Use collision shape to find a valid ground spot, if appropriate
	if (CollisionShape.ShapeType != ECollisionShape::Line)
	{
		ReturnHitResult.Location.Z += CollisionHeightOffset; //Rise up out of the ground
		TraceStart = InSourceActor->GetActorLocation();
		TraceEnd = ReturnHitResult.Location;
		TraceStart.Z += CollisionHeightOffset;
		bLastTraceWasGood = AdjustCollisionResultForShape(TraceStart, TraceEnd, Params, ReturnHitResult);
		if (bLastTraceWasGood)
		{
			ReturnHitResult.Location.Z -= CollisionHeightOffset; //Undo the artificial height adjustment
		}
	}

	if (auto* LocalReticleActor = ReticleActor.Get())
	{
		LocalReticleActor->SetIsTargetValid(bLastTraceWasGood);
		LocalReticleActor->SetActorLocation(ReturnHitResult.Location);
	}

	// Reset the trace start so the target data uses the correct origin
	ReturnHitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();

	return ReturnHitResult;
}

void ACTRLGas_GroundTraceFromCamera::Tick(float const DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bHasTicked)
	{
		bHasTicked = true;
	}
}

bool ACTRLGas_GroundTraceFromCamera::ShouldProduceTargetData() const
{
	return Super::ShouldProduceTargetData();
}

bool ACTRLGas_GroundTraceFromCamera::IsConfirmTargetingAllowed()
{
	if (!bHasTicked)
	{
		return true;
	}
	return Super::IsConfirmTargetingAllowed();
}
