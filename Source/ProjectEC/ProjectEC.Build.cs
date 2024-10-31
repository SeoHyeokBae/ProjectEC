// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectEC : ModuleRules
{
	public ProjectEC(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.AddRange(new string[] { "ProjectEC" });
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
