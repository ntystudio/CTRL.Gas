// Copyright Epic Games, Inc. All Rights Reserved.

#include "UseGASEffectContext.h"

#include "LyraAbilitySourceInterface.h"

#include "Engine/HitResult.h"
#include "Engine/NetSerialization.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"

#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include "Abilities/GameplayAbility.h"

#include "UseGAS/UseGAS.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UseGASEffectContext)

class FArchive;

FUseGASEffectContext* FUseGASEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	auto* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(ThisStruct::StaticStruct()))
	{
		return static_cast<ThisStruct*>(BaseEffectContext);
	}

	return nullptr;
}

bool FUseGASEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Super::NetSerialize(Ar, Map, bOutSuccess);

	// Not serialized for post-activation use:
	// CartridgeID

	return true;
}

#if UE_WITH_IRIS
namespace UE::Net
{
	// Forward to FGameplayEffectContextNetSerializer
	// Note: If FUseGASEffectContext::NetSerialize() is modified, a custom NetSerializesr must be implemented as the current fallback will no longer be sufficient.
	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(UseGASEffectContext, FGameplayEffectContextNetSerializer);
}
#endif

void FUseGASEffectContext::SetAbilitySource(ILyraAbilitySourceInterface const* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<UObject const>(InObject));
	//SourceLevel = InSourceLevel;
}

ILyraAbilitySourceInterface const* FUseGASEffectContext::GetAbilitySource() const
{
	return Cast<ILyraAbilitySourceInterface>(AbilitySourceObject.Get());
}

// ReSharper disable once CppEnforceCVQualifiersPlacement
const UPhysicalMaterial* FUseGASEffectContext::GetPhysicalMaterial() const
{
	if (FHitResult const* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}
