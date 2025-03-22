// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#include "CTRLTargetDataFilters.h"

#include "Algo/AllOf.h"
#include "Algo/AnyOf.h"

bool FCTRLTargetDataFilterByGameplayTags::FilterPassesForActor(AActor const* ActorToBeFiltered) const
{
	if (!Super::FilterPassesForActor(ActorToBeFiltered)) return false;
	if (!bUseQuery && !ensure(MatchTags.IsValid())) return false;
	if (bUseQuery && !ensure(!Query.IsEmpty())) return false;

	FGameplayTagQuery QueryToUse;
	if (bUseQuery)
	{
		QueryToUse = Query;
	}
	else
	{
		QueryToUse = bMatchAny ? FGameplayTagQuery::MakeQuery_MatchAnyTags(MatchTags) : FGameplayTagQuery::MakeQuery_MatchAllTags(MatchTags);
	}

	auto const ActorTags = UCTRLGasUtils::GetGameplayTags(ActorToBeFiltered);
	bool const bMatchesQuery = QueryToUse.Matches(ActorTags);
	return bReverseFilter ^ bMatchesQuery;
}

bool FCTRLTargetDataFilterByMultiple::FilterPassesForActor(AActor const* ActorToBeFiltered) const
{
	if (!Super::FilterPassesForActor(ActorToBeFiltered)) return false;
	auto FilterWithHandle = [ActorToBeFiltered](FGameplayTargetDataFilterHandle const& FilterHandle) -> bool
	{
		if (!ensure(FilterHandle.Filter.IsValid())) return false;
		return FilterHandle.FilterPassesForActor(ActorToBeFiltered);
	};
	bool const bMatchesAll = bRequireAllFiltersPass ? Algo::AllOf(Filters, FilterWithHandle) : Algo::AnyOf(Filters, FilterWithHandle);
	return bReverseFilter ^ bMatchesAll;
}

FGameplayTargetDataFilterHandle UCTRLTargetDataFilterUtils::MakeFilterByMultipleHandle(TArray<FGameplayTargetDataFilterHandle> Filters, AActor* FilterActor, bool bRequireAll)
{
	FGameplayTargetDataFilter* NewFilter = new FCTRLTargetDataFilterByMultiple(Filters, bRequireAll);
	NewFilter->InitializeFilterContext(FilterActor);
	FGameplayTargetDataFilterHandle FilterHandle;
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return MoveTemp(FilterHandle);
}

FGameplayTargetDataFilterHandle UCTRLTargetDataFilterUtils::MakeFilterByCombinedHandle(
	FGameplayTargetDataFilterHandle const& FilterA,
	FGameplayTargetDataFilterHandle const& FilterB,
	AActor* FilterActor,
	bool const bRequireAll
)
{
	TArray<FGameplayTargetDataFilterHandle> CombinedFilters;
	CombinedFilters.Add(FilterA);
	CombinedFilters.Add(FilterB);
	return MakeFilterByMultipleHandle(CombinedFilters, FilterActor, bRequireAll);
}

FGameplayTargetDataFilterHandle UCTRLTargetDataFilterUtils::MakeFilterByInterfaceHandle(FCTRLTargetDataFilterByInterface Filter, AActor* FilterActor)
{
	FGameplayTargetDataFilter* NewFilter = new FCTRLTargetDataFilterByInterface(Filter);
	NewFilter->InitializeFilterContext(FilterActor);
	FGameplayTargetDataFilterHandle FilterHandle;
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return MoveTemp(FilterHandle);
}

FGameplayTargetDataFilterHandle UCTRLTargetDataFilterUtils::MakeFilterByGameplayTagsHandle(FCTRLTargetDataFilterByGameplayTags Filter, AActor* FilterActor)
{
	FGameplayTargetDataFilter* NewFilter = new FCTRLTargetDataFilterByGameplayTags(Filter);
	NewFilter->InitializeFilterContext(FilterActor);
	FGameplayTargetDataFilterHandle FilterHandle;
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return MoveTemp(FilterHandle);
}
