using UnrealBuildTool;
using System.Collections.Generic;

public class YJJActionCppUE5EditorTarget : TargetRules
{
	public YJJActionCppUE5EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		ExtraModuleNames.Add("YJJActionCppUE5");
	}
}
