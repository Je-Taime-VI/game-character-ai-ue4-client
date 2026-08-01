using UnrealBuildTool;

public class GameCharacterAI_UE4 : ModuleRules
{
    public GameCharacterAI_UE4(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "Slate",
            "SlateCore",
            "HTTP",
            "Json",
            "JsonUtilities"
        });
    }
}
