// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

using UnrealBuildTool;

public class GamemakinLinter : ModuleRules
{
    public GamemakinLinter(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;		
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
                "Engine",
				"Linter"
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
            {

			}
		);
    }
}
