using UnrealBuildTool;

public class YJJActionCppUE5 : ModuleRules
{
	public YJJActionCppUE5(ReadOnlyTargetRules Target) : base(Target)
	{
		CppStandard = CppStandardVersion.Cpp20;
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Unity 빌드 병합을 줄여 cl.exe 메모리 피크를 낮춘다.
		bUseUnity = false;

		PublicIncludePaths.Add(ModuleDirectory);    // ../ 상대 경로 사용을 줄인다.

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
