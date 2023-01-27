// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

using UnrealBuildTool;

public class Linter : ModuleRules
{
    public Linter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
				"AppFramework",
				"InputCore",
                "UnrealEd",
                "GraphEditor",
                "AssetTools",
                "EditorStyle",
                "BlueprintGraph",
                "PropertyEditor",
                "LauncherPlatform",
                "Projects",
				"DesktopPlatform",
				"Json",
                "UATHelper"
				// ... add private dependencies that you statically link with here ...	
			}
		);

        PublicIncludePathModuleNames.Add("Launch");

#if UE_4_20_OR_LATER
		PublicDefinitions.Add("UE_4_20_OR_LATER=1");
#endif

    }
}
