// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffectTypes.h"
#include "UseGASEffectContext.generated.h"

class ILyraAbilitySourceInterface;
class AActor;
class FArchive;
class UObject;
class UPhysicalMaterial;

USTRUCT()
struct USEGAS_API FUseGASEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	FUseGASEffectContext(): Super() {}

	// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
	using ThisStruct = FUseGASEffectContext;

	FUseGASEffectContext(AActor* InInstigator, AActor* InEffectCauser)
		: Super(InInstigator, InEffectCauser) {}

	/** Returns the wrapped FUseGASEffectContext from the handle, or nullptr if it doesn't exist or is the wrong type */
	static ThisStruct* ExtractEffectContext(FGameplayEffectContextHandle Handle);

	/** Sets the object used as the ability source */
	void SetAbilitySource(ILyraAbilitySourceInterface const* InObject, float InSourceLevel);

	/** Returns the ability source interface associated with the source object. Only valid on the authority. */
	ILyraAbilitySourceInterface const* GetAbilitySource() const;

	virtual FGameplayEffectContext* Duplicate() const override
	{
		FUseGASEffectContext* NewContext = new FUseGASEffectContext();
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
	/** Ability Source object (should implement ILyraAbilitySourceInterface). NOT replicated currently */
	UPROPERTY()
	TWeakObjectPtr<UObject const> AbilitySourceObject;
};

template <>
struct TStructOpsTypeTraits<FUseGASEffectContext> : public TStructOpsTypeTraitsBase2<FUseGASEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
