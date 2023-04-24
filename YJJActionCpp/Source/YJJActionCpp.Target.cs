using UnrealBuildTool;
using System.Collections.Generic;

public class YJJActionCppTarget : TargetRules
{
	public YJJActionCppTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "YJJActionCpp" } );
	}
}
