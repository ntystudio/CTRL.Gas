// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUseGAS, Log, All);

#define USEGAS_LOG(Verbosity, ...) UE_LOG(LogUseGAS, Verbosity, ##__VA_ARGS__)

class FUseGASModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
