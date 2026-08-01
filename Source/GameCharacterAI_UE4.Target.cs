using UnrealBuildTool;
using System.Collections.Generic;

public class GameCharacterAI_UE4Target : TargetRules
{
    public GameCharacterAI_UE4Target(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.Add("GameCharacterAI_UE4");
    }
}
