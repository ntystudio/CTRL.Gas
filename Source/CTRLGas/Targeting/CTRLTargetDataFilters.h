// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "Abilities/GameplayAbilityTargetDataFilter.h"

#include "CTRLGas/CTRLGasUtils.h"

#include "UObject/Object.h"

#include "CTRLTargetDataFilters.generated.h"

/**
 * Filter that checks if an actor implements a specific interface.
 */
USTRUCT(BlueprintType, DisplayName="Filter By Interface [CTRL]", Category="CTRL|Gas|Targeting")
struct CTRLGAS_API FCTRLTargetDataFilterByInterface : public FGameplayTargetDataFilter
{
	GENERATED_BODY()

public:
	/** Interface actors must implement to pass the filter. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	TSubclassOf<UInterface> Interface = nullptr;

	virtual bool FilterPassesForActor(AActor const* ActorToBeFiltered) const override
	{
		if (!Super::FilterPassesForActor(ActorToBeFiltered)) return false;
		if (!IsValid(Interface)) return false;

		return bReverseFilter ^ ActorToBeFiltered->GetClass()->ImplementsInterface(Interface);
	}
};

USTRUCT(BlueprintType, DisplayName="Filter By Gameplay Tags [CTRL]", Category="CTRL|Gas|Targeting")
struct CTRLGAS_API FCTRLTargetDataFilterByGameplayTags : public FGameplayTargetDataFilter
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	bool bUseQuery = false;

	/** Interface actors must implement to pass the filter. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	FGameplayTagQuery Query;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	FGameplayTagContainer MatchTags;

	// otherwise uses match all
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	bool bMatchAny = false;

	virtual bool FilterPassesForActor(AActor const* ActorToBeFiltered) const override;
};

USTRUCT(BlueprintType, DisplayName="Filter By Multiple [CTRL]", Category="CTRL|Gas|Targeting")
struct CTRLGAS_API FCTRLTargetDataFilterByMultiple : public FGameplayTargetDataFilter
{
	GENERATED_BODY()

public:
	FCTRLTargetDataFilterByMultiple() = default;

	FCTRLTargetDataFilterByMultiple(TArray<FGameplayTargetDataFilterHandle> InFilters, bool bRequireAll = true)
		: FGameplayTargetDataFilter()
	{
		bRequireAllFiltersPass = bRequireAll;
		Filters = InFilters;
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	TArray<FGameplayTargetDataFilterHandle> Filters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = Filter)
	bool bRequireAllFiltersPass = true;
	virtual bool FilterPassesForActor(AActor const* ActorToBeFiltered) const override;
};

UCLASS(DisplayName="Target Data Filter Utils [CTRL]", Category="CTRL|Gas|Targeting")
class CTRLGAS_API UCTRLTargetDataFilterUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "CTRL|Gas|Targeting", DisplayName="Make 'Filter By Multiple' Handle [CTRL]")
	static FGameplayTargetDataFilterHandle MakeFilterByMultipleHandle(TArray<FGameplayTargetDataFilterHandle> Filters, AActor* FilterActor, bool bRequireAll = true);

	UFUNCTION(BlueprintPure, Category = "CTRL|Gas|Targeting", DisplayName="Make 'Filter By Combined' Handle [CTRL]")
	static FGameplayTargetDataFilterHandle MakeFilterByCombinedHandle(FGameplayTargetDataFilterHandle const& FilterA, FGameplayTargetDataFilterHandle const& FilterB, AActor* FilterActor, bool bRequireAll = true);

	UFUNCTION(BlueprintPure, Category = "CTRL|Gas|Targeting", DisplayName="Make 'Filter By Interface' Handle [CTRL]")
	static FGameplayTargetDataFilterHandle MakeFilterByInterfaceHandle(FCTRLTargetDataFilterByInterface Filter, AActor* FilterActor);

	UFUNCTION(BlueprintPure, Category = "CTRL|Gas|Targeting", DisplayName="Make 'Filter By GameplayTags' Handle [CTRL]")
	static FGameplayTargetDataFilterHandle MakeFilterByGameplayTagsHandle(FCTRLTargetDataFilterByGameplayTags Filter, AActor* FilterActor);
};
