// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

#include "CTRLGasPhysicalMaterialWithTags.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, DisplayName="Physical Material With Tags [CTRL]", Category="CTRL|PhysicalMaterial")
class CTRLGAS_API UCTRLGasPhysicalMaterialWithTags : public UPhysicalMaterial
{
	GENERATED_BODY()

public:
	UCTRLGasPhysicalMaterialWithTags(FObjectInitializer const& ObjectInitializer = FObjectInitializer::Get());

	// A container of gameplay tags that game code can use to reason about this physical material
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=PhysicalProperties)
	FGameplayTagContainer Tags;
};
