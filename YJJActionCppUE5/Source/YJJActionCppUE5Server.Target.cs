using UnrealBuildTool;
using System.Collections.Generic;

public class YJJActionCppUE5ServerTarget : TargetRules
{
	public YJJActionCppUE5ServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("YJJActionCppUE5");
	}
}
