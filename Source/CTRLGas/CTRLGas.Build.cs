// Copyright Epic Games, Inc. All Rights Reserved.

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
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"PhysicsCore",
				"Slate",
				"SlateCore",
			}
		);

		SetupGameplayDebuggerSupport(Target);
		SetupIrisSupport(Target);
	}
}