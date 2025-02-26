// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"

#include "CTRLGasAbilitySourceInterface.generated.h"

class UObject;
class UPhysicalMaterial;
struct FGameplayTagContainer;

/** Base interface for anything acting as an ability calculation source */
UINTERFACE()
class UCTRLGasAbilitySourceInterface : public UInterface
{
	GENERATED_BODY()
};

class ICTRLGasAbilitySourceInterface
{
	GENERATED_BODY()

	/**
	 * Compute the multiplier for effect falloff with distance
	 * 
	 * @param Distance			Distance from source to target for ability calculations (distance bullet traveled for a gun, etc...)
	 * @param SourceTags		Aggregated Tags from the source
	 * @param TargetTags		Aggregated Tags on the target
	 * 
	 * @return Multiplier to apply to the base attribute value due to distance
	 */
	virtual float GetDistanceAttenuation(float Distance, FGameplayTagContainer const* SourceTags = nullptr, FGameplayTagContainer const* TargetTags = nullptr) const = 0;

	virtual float GetPhysicalMaterialAttenuation(UPhysicalMaterial const* PhysicalMaterial, FGameplayTagContainer const* SourceTags = nullptr, FGameplayTagContainer const* TargetTags = nullptr) const = 0;
};
