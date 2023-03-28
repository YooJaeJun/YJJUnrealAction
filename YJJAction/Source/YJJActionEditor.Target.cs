using UnrealBuildTool;
using System.Collections.Generic;

public class YJJActionEditorTarget : TargetRules
{
	public YJJActionEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "YJJAction" } );
	}
}
