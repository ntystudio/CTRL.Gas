// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"

#include "Abilities/GameplayAbilityTypes.h"

#include "Animation/AnimNotifies/AnimNotify.h"

#include "CTRLGasGameplayEventNotify.generated.h"

/**
 * Emit a GameplayEvent when this notify is triggered.
 */
UCLASS(Blueprintable, BlueprintType, AutoExpandCategories=("Event"), meta=(DisplayName="Gameplay Event Notify [CTRLGas]"))
class CTRLGAS_API UCTRLGasGameplayEventNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Event", meta=(ShowOnlyInnerProperties))
	FGameplayEventData EventData;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, FAnimNotifyEventReference const& EventReference) override;

#if WITH_EDITOR
	virtual void ValidateAssociatedAssets() override;
#endif
};
