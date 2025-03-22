// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CTRLTargetDataFilters.h"

#include "Abilities/GameplayAbilityWorldReticle.h"
#include "ACTRLGameplayAbilityWorldReticle.generated.h"

UCLASS()
class CTRLGAS_API AACTRLGameplayAbilityWorldReticle : public AGameplayAbilityWorldReticle
{
	GENERATED_BODY()

public:
	AACTRLGameplayAbilityWorldReticle();
		
	UPROPERTY(Transient, DuplicateTransient)
	TWeakObjectPtr<AActor> TargetedActor;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AActor* GetTargetedActor() const
	{
		return TargetedActor.Get();
	}

	virtual void Tick(float DeltaSeconds) override;
};
