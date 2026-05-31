using UnrealBuildTool;

public class YJJActionCppUE5Editor : ModuleRules
{
	public YJJActionCppUE5Editor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		CppStandard = CppStandardVersion.Cpp20;
		bUseUnity = false;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"UMG",
			"UMGEditor",
			"Kismet",
			"BlueprintGraph",
			"BlueprintEditorLibrary",
			"AssetRegistry",
			"YJJActionCppUE5",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"KismetCompiler",
			"EditorSubsystem",
		});
	}
}
