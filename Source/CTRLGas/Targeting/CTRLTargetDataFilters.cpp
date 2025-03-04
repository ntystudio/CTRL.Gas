// Fill out your copyright notice in the Description page of Project Settings.

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
