using UnrealBuildTool;

public class YJJActionCppUE5 : ModuleRules
{
	public YJJActionCppUE5(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Lowers per-cl.exe memory (fewer .cpp merged per unity unit). Helps MSVC PCH / internal heap OOM.
		bUseUnity = false;

		PublicIncludePaths.Add(ModuleDirectory);    // flatten includes vs ../ chains

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
			"EngineCameras"
		});
	}
}
