// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UseGAS : ModuleRules
{
	public UseGAS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"GameplayAbilities",
				"GameplayTasks",
				"PhysicsCore",
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
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