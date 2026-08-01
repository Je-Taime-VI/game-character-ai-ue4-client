using UnrealBuildTool;
using System.Collections.Generic;

public class GameCharacterAI_UE4EditorTarget : TargetRules
{
    public GameCharacterAI_UE4EditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.Add("GameCharacterAI_UE4");
    }
}
