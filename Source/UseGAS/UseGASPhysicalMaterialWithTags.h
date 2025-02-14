// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

#include "UseGASPhysicalMaterialWithTags.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class USEGAS_API UUseGASPhysicalMaterialWithTags : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UUseGASPhysicalMaterialWithTags(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());

	// A container of gameplay tags that game code can use to reason about this physical material
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PhysicalProperties)
	FGameplayTagContainer Tags;
};
