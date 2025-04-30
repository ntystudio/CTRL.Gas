// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask.h"

#include "CTRLGasAbilityTask.generated.h"

#define END_WITH_ERROR(Verbosity, ...) \
([&]() -> void { \
	UE_LOG(LogTemp, Verbosity, ##__VA_ARGS__); \
	if (!ShouldBroadcastAbilityTaskDelegates()) { return; } \
	OnError.Broadcast(); \
	EndTask(); \
}())

class UCTRLAbilitySystemComponent;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, DisplayName="Ability Task [CTRLGas]")
class CTRLGAS_API UCTRLGasAbilityTask : public UAbilityTask
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUGAbilityTaskDelegate);

	UPROPERTY(BlueprintAssignable)
	FUGAbilityTaskDelegate OnActivated;

	UPROPERTY(BlueprintAssignable)
	FUGAbilityTaskDelegate OnCompleted;

	UPROPERTY(BlueprintAssignable)
	FUGAbilityTaskDelegate OnError;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UCTRLAbilitySystemComponent* GetASC() const;

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
};
