// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Path_NoTopLevel.h"
#include "LintRuleSet.h"
#include "LinterNamingConvention.h"
#include "HAL/FileManager.h"

ULintRule_Path_NoTopLevel::ULintRule_Path_NoTopLevel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ZeroTopLevelFoldersRecommendedAction = NSLOCTEXT("Linter", "LintRule_Path_NoTopLevel_ZeroTopLevelFolders", "There appears to be no top level folders. Please put your assets in a top level folder.");
	PleaseUseThisFolderRecommendedAction = NSLOCTEXT("Linter", "LintRule_Path_NoTopLevel_PleaseUseThisFolder", "Please move this asset into a top level folder. Maybe \"{0}\"?");
}

bool ULintRule_Path_NoTopLevel::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	FString PathName = ObjectToLint->GetPathName();
	TArray<FString> PathElements;
	PathName.ParseIntoArray(PathElements, TEXT("/"), true);

	// Report issue with top assets not in a top level folder
	if (PathElements.Num() == 1)
	{
		FText RecommendedAction;

		// This is really slow to do for every single asset that fails to be in a top level folder
		// But doing it here makes the code base a lot cleaner and easier to follow for now.
		{
			// Determine content sub directory structure for project organization based rules
			TArray<FString> Subdirectories;
			IFileManager::Get().FindFiles(Subdirectories, *(FPaths::ProjectContentDir() / TEXT("*")), false, true);
			Subdirectories.Remove(TEXT("Collections"));
			Subdirectories.Remove(TEXT("Developers"));

			if (Subdirectories.Num() == 0)
			{
				RecommendedAction = ZeroTopLevelFoldersRecommendedAction;
			}
			else
			{
				FString MostPopulatedContentDir;
				int32 FileCount = 0;
				for (FString Subdirectory : Subdirectories)
				{
					TArray<FString> FileNames;
					IFileManager::Get().FindFilesRecursive(FileNames, *(FPaths::ProjectContentDir() / Subdirectory), TEXT("*"), true, false, false);
					if (FileNames.Num() > FileCount)
					{
						FileCount = FileNames.Num();
						MostPopulatedContentDir = Subdirectory;
					}
				}

				RecommendedAction = FText::FormatOrdered(PleaseUseThisFolderRecommendedAction, FText::FromString(MostPopulatedContentDir));
			}
		}

		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), RecommendedAction));
		return false;
	}
	
	return true;
}