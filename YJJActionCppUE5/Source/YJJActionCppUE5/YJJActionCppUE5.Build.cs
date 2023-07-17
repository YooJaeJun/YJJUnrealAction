using UnrealBuildTool;

public class YJJActionCppUE5 : ModuleRules
{
	public YJJActionCppUE5(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(ModuleDirectory);    // ../ ����� ���ֱ� ����

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
