// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#include "CTRLGas/Targeting/CTRLTargetDataFilters.h"

FGameplayTargetDataFilterHandle UCTRLTargetDataFilterUtils::MakeFilterByInterfaceHandle(FCTRLTargetDataFilterByInterface Filter, AActor* FilterActor)
{
	FGameplayTargetDataFilterHandle FilterHandle;
	FGameplayTargetDataFilter* NewFilter = new FCTRLTargetDataFilterByInterface(Filter);
	NewFilter->InitializeFilterContext(FilterActor);
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return FilterHandle;
}

FGameplayTargetDataFilterHandle UCTRLTargetDataFilterUtils::MakeFilterByGameplayTagsHandle(FCTRLTargetDataFilterByGameplayTags Filter, AActor* FilterActor)
{
	FGameplayTargetDataFilterHandle FilterHandle;
	FGameplayTargetDataFilter* NewFilter = new FCTRLTargetDataFilterByGameplayTags(Filter);
	NewFilter->InitializeFilterContext(FilterActor);
	FilterHandle.Filter = TSharedPtr<FGameplayTargetDataFilter>(NewFilter);
	return FilterHandle;
}
