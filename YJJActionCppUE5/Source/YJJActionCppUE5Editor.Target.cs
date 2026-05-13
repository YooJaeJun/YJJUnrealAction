using UnrealBuildTool;
using System.Collections.Generic;

public class YJJActionCppUE5EditorTarget : TargetRules
{
	public YJJActionCppUE5EditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		// V2 differs from UnrealEditor shared defaults; Unique avoids UBT "common build products" conflict.
		BuildEnvironment = TargetBuildEnvironment.Unique;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("YJJActionCppUE5");
	}
}
