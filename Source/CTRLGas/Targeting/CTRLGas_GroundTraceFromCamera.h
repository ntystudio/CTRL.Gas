// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"

#include "Abilities/GameplayAbilityTargetActor_GroundTrace.h"

#include "CTRLGas_GroundTraceFromCamera.generated.h"

UCLASS(BlueprintType)
class CTRLGAS_API ACTRLGas_GroundTraceFromCamera : public AGameplayAbilityTargetActor_GroundTrace
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldProduceTargetData() const override;

protected:
	bool bHasTicked = false; // Allow IsConfirmTargetingAllowed to be true before first tick to support instant confirm with ground trace.

	virtual bool IsConfirmTargetingAllowed() override;
	virtual FHitResult PerformTrace(AActor* InSourceActor) override;
};
