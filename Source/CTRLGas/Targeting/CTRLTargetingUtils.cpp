// Fill out your copyright notice in the Description page of Project Settings.

#include "CTRLGas/Targeting/CTRLTargetingUtils.h"

#include "Abilities/GameplayAbilityTargetTypes.h"

AActor* UCTRLTargetingUtils::GetActorFromTargetData(FGameplayAbilityTargetDataHandle const& TargetData, TSubclassOf<AActor> const ActorClass, ECTRLTargetDataValid& ActorIsValid, int32 const DataIndex, int32 const ActorIndex)
{
	ActorIsValid = ECTRLTargetDataValid::NotValid;
	if (!TargetData.Data.IsValidIndex(DataIndex)) return nullptr;
	auto const* DataAtIndex = TargetData.Data[0].Get();
	if (!DataAtIndex) return nullptr;
	auto WeakArray = DataAtIndex->GetActors();
	if (!WeakArray.IsValidIndex(ActorIndex)) return nullptr;
	auto const Actor = WeakArray[ActorIndex].Get();
	if (!IsValid(Actor)) return nullptr;
	if (!ActorClass || (ActorClass && Actor->IsA(ActorClass)))
	{
		ActorIsValid = ECTRLTargetDataValid::Valid;
		return Actor;
	}
	return nullptr;
}
