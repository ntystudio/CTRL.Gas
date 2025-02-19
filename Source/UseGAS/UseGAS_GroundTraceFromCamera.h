// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"

#include "Abilities/GameplayAbilityTargetActor_GroundTrace.h"

#include "UseGAS_GroundTraceFromCamera.generated.h"

UCLASS(BlueprintType)
class USEGAS_API AUseGAS_GroundTraceFromCamera : public AGameplayAbilityTargetActor_GroundTrace
{
	GENERATED_BODY()

protected:
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldProduceTargetData() const override;

protected:
	virtual bool IsConfirmTargetingAllowed() override;

	bool bHasTicked = false; // Allow IsConfirmTargetingAllowed to be true before first tick to support instant confirm with ground trace.
};
