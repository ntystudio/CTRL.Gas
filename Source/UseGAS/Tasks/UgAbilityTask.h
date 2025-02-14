// SPDX-FileCopyrightText: 2025 NTY.studio

#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask.h"

#include "UgAbilityTask.generated.h"

#define END_WITH_ERROR(Verbosity, ...) \
([&]() -> void { \
	UE_LOG(LogTemp, Verbosity, ##__VA_ARGS__); \
	if (!ShouldBroadcastAbilityTaskDelegates()) { return; } \
	OnError.Broadcast(); \
	EndTask(); \
}())

class UUseGASAbilitySystemComponent;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, BlueprintType, DisplayName="Ability Task [Ug]")
class USEGAS_API UUgAbilityTask : public UAbilityTask
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
	UUseGASAbilitySystemComponent* GetASC() const;

	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;
};
