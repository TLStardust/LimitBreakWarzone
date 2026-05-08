// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LimitBreakWarzone : ModuleRules
{
	public LimitBreakWarzone(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayAbilities", "GameplayTags", "GameplayTasks", "UMG" });
	}
}
