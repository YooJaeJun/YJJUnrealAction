using UnrealBuildTool;
using System.Collections.Generic;

public class YJJActionCppUE5Target : TargetRules
{
	public YJJActionCppUE5Target(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
		ExtraModuleNames.Add("YJJActionCppUE5");
	}
}
