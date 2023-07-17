using UnrealBuildTool;

public class YJJActionCppUE5 : ModuleRules
{
	public YJJActionCppUE5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(ModuleDirectory);    // ../ 상대경로 없애기 위함

        PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"HeadMountedDisplay",
            "EnhancedInput",
            "UMG",
            "Niagara",
            "AIModule",
            "GameplayTasks",
            "NavigationSystem",
            "GameplayCameras"
        });
	}
}
