// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

#include "CTRLGas.h"

#define LOCTEXT_NAMESPACE "FCTRLGasModule"

DEFINE_LOG_CATEGORY(LogCTRLGas);

void FCTRLGasModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCTRLGasModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCTRLGasModule, CTRLGas)