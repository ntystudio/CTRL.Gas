// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#pragma once

#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCTRLGas, Log, All);

#define CTRL_GAS_LOG(Verbosity, ...) UE_LOG(LogCTRLGas, Verbosity, ##__VA_ARGS__)

class FCTRLGasModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
