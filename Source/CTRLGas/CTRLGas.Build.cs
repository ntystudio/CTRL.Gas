// SPDX-FileCopyrightText: © 2025 NTY.studio
// SPDX-License-Identifier: MIT

using UnrealBuildTool;

public class CTRLGas : ModuleRules
{
	public CTRLGas(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new[]
			{
				"Core",
				"GameplayAbilities",
				"GameplayTasks",
				"PhysicsCore",
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new[]
			{
				"CTRLCore",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"PhysicsCore",
				"Slate",
				"SlateCore",
				"WorldConditions"
			}
		);

		SetupGameplayDebuggerSupport(Target);
		SetupIrisSupport(Target);
	}
}