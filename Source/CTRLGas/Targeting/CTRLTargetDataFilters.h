// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "WorldConditionQuery.h"

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
		if (!IsValid(Interface)) return true;

		return ActorToBeFiltered->GetClass()->ImplementsInterface(Interface);
	}
};

USTRUCT(BlueprintType, DisplayName="Filter By Gameplay Tags [CTRL]", Category="CTRL|Gas|Targeting")
struct CTRLGAS_API FCTRLTargetDataFilterByGameplayTags : public FGameplayTargetDataFilter
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true, InlineEditConditionToggle), Category = Filter)
	bool bUseQuery = false;

	/** Interface actors must implement to pass the filter. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true, EditCondition="bUseQuery"), Category = Filter)
	FGameplayTagQuery Query;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true, EditCondition="!bUseQuery"), Category = Filter)
	FGameplayTagContainer MatchTags;

	// otherwise uses match all
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true, EditCondition="!bUseQuery"), Category = Filter)
	bool bMatchAny = false;

	virtual bool FilterPassesForActor(AActor const* ActorToBeFiltered) const override
	{
		if (!Super::FilterPassesForActor(ActorToBeFiltered)) return false;
		if (!bUseQuery && !MatchTags.IsValid()) return false;

		FGameplayTagQuery QueryToUse;
		if (bUseQuery)
		{
			QueryToUse = Query;
		}
		else
		{
			QueryToUse = bMatchAny ? FGameplayTagQuery::MakeQuery_MatchAnyTags(MatchTags) : FGameplayTagQuery::MakeQuery_MatchAllTags(MatchTags);
		}

		if (QueryToUse.IsEmpty()) return false;

		auto const ActorTags = UCTRLGasUtils::GetGameplayTags(ActorToBeFiltered);
		return QueryToUse.Matches(ActorTags);
	}
};

UCLASS(DisplayName="Target Data Filter Utils [CTRL]", Category="CTRL|Gas|Targeting")
class CTRLGAS_API UCTRLTargetDataFilterUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "CTRL|Gas|Targeting", DisplayName="Make Filter By Interface Handle [CTRL]")
	static FGameplayTargetDataFilterHandle MakeFilterByInterfaceHandle(FCTRLTargetDataFilterByInterface Filter, AActor* FilterActor);

	UFUNCTION(BlueprintPure, Category = "CTRL|Gas|Targeting", DisplayName="Make Filter By GameplayTags Handle [CTRL]")
	static FGameplayTargetDataFilterHandle MakeFilterByGameplayTagsHandle(FCTRLTargetDataFilterByGameplayTags Filter, AActor* FilterActor);
};
