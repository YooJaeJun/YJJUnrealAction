using UnrealBuildTool;

public class DebugLog : ModuleRules
{
	public DebugLog(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(ModuleDirectory);

        PublicDependencyModuleNames.Add("Core");

		PrivateDependencyModuleNames.Add("YJJActionCpp");

        PrivateDependencyModuleNames.Add("CoreUObject");
        PrivateDependencyModuleNames.Add("Engine");
        PrivateDependencyModuleNames.Add("Slate");
        PrivateDependencyModuleNames.Add("SlateCore");
        PrivateDependencyModuleNames.Add("GameplayDebugger");
        PrivateDependencyModuleNames.Add("DesktopPlatform");
        PrivateDependencyModuleNames.Add("MainFrame");
        PrivateDependencyModuleNames.Add("EditorStyle");
        PrivateDependencyModuleNames.Add("UnrealEd");
	}
}
