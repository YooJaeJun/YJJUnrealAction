using UnrealBuildTool;
using System.Collections.Generic;

public class YJJActionTarget : TargetRules
{
	public YJJActionTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "YJJAction" } );
	}
}
