using UnrealBuildTool;
using System.Collections.Generic;

public class YJJActionCppEditorTarget : TargetRules
{
	public YJJActionCppEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "YJJActionCpp" } );
	}
}
