// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "GameplayEffectTypes.h"
#include "CTRLGasEffectContext.generated.h"

class ICTRLGasAbilitySourceInterface;
class AActor;
class FArchive;
class UObject;
class UPhysicalMaterial;

USTRUCT()
struct CTRLGAS_API FCTRLGasEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FCTRLGasEffectContext(): Super() {}

	// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
	using ThisStruct = FCTRLGasEffectContext;

	FCTRLGasEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: Super(InInstigator, InEffectCauser) {}

	/** Returns the wrapped FCTRLGasEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
	static ThisStruct* ExtractEffectContext(FGameplayEffectContextHandle Handle);

	/** Sets the object used as the ability source */
	void SetAbilitySource(ICTRLGasAbilitySourceInterface const* InObject, float InSourceLevel);

	/** Returns the ability source interface associated with the source object. Only valid on the authority. */
	ICTRLGasAbilitySourceInterface const* GetAbilitySource() const;

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FCTRLGasEffectContext* NewContext = new FCTRLGasEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	virtual UScriptStruct* GetScriptStruct() const override
	{
		return ThisStruct::StaticStruct();
	}

	/** Overridden to serialize new fields */
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	/** Returns the physical material from the hit result if there is one */
	// ReSharper disable once CppEnforceCVQualifiersPlacement
	const UPhysicalMaterial* GetPhysicalMaterial() const;

public:
	/** ID to allow the identification of multiple bullets that were part of the same cartridge */
	UPROPERTY()
	int32 CartridgeID = -1;

protected:
	/** Ability Source object (should implement ICTRLGasAbilitySourceInterface). NOT replicated currently */
	UPROPERTY()
	TWeakObjectPtr<UObject const> AbilitySourceObject;
};

template <>
struct TStructOpsTypeTraits<FCTRLGasEffectContext> : public TStructOpsTypeTraitsBase2<FCTRLGasEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
