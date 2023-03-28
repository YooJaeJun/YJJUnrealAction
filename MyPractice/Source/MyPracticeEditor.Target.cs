using UnrealBuildTool;
using System.Collections.Generic;

public class MyPracticeEditorTarget : TargetRules
{
	public MyPracticeEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "MyPractice" } );
	}
}
