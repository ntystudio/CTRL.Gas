// Copyright Epic Games, Inc. All Rights Reserved.

#include "CTRLGasEffectContext.h"

#include "CTRLGasAbilitySourceInterface.h"

#include "Engine/HitResult.h"
#include "Engine/NetSerialization.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"

#include "Serialization/GameplayEffectContextNetSerializer.h"
#endif

#include "Abilities/GameplayAbility.h"

#include "CTRLGas/CTRLGas.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CTRLGasEffectContext)

class FArchive;

FCTRLGasEffectContext* FCTRLGasEffectContext::ExtractEffectContext(struct FGameplayEffectContextHandle Handle)
{
	auto* BaseEffectContext = Handle.Get();
	if ((BaseEffectContext != nullptr) && BaseEffectContext->GetScriptStruct()->IsChildOf(ThisStruct::StaticStruct()))
	{
		return static_cast<ThisStruct*>(BaseEffectContext);
	}

	return nullptr;
}

bool FCTRLGasEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
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
	// Note: If FCTRLGasEffectContext::NetSerialize() is modified, a custom NetSerializesr must be implemented as the current fallback will no longer be sufficient.
	UE_NET_IMPLEMENT_FORWARDING_NETSERIALIZER_AND_REGISTRY_DELEGATES(CTRLGasEffectContext, FGameplayEffectContextNetSerializer);
}
#endif

void FCTRLGasEffectContext::SetAbilitySource(ICTRLGasAbilitySourceInterface const* InObject, float InSourceLevel)
{
	AbilitySourceObject = MakeWeakObjectPtr(Cast<UObject const>(InObject));
	//SourceLevel = InSourceLevel;
}

ICTRLGasAbilitySourceInterface const* FCTRLGasEffectContext::GetAbilitySource() const
{
	return Cast<ICTRLGasAbilitySourceInterface>(AbilitySourceObject.Get());
}

// ReSharper disable once CppEnforceCVQualifiersPlacement
const UPhysicalMaterial* FCTRLGasEffectContext::GetPhysicalMaterial() const
{
	if (FHitResult const* HitResultPtr = GetHitResult())
	{
		return HitResultPtr->PhysMaterial.Get();
	}
	return nullptr;
}
