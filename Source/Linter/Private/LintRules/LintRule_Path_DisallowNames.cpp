// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Path_DisallowNames.h"
#include "LintRuleSet.h"
#include "LinterNamingConvention.h"
#include "HAL/FileManager.h"

ULintRule_Path_DisallowNames::ULintRule_Path_DisallowNames(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RecommendedAction = NSLOCTEXT("Linter", "LintRule_Path_DisallowNames_ChangeName", "Please rename \"{0}\" to an allowed name.");
}

bool ULintRule_Path_DisallowNames::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	FString PathName = ObjectToLint->GetPathName();
	TArray<FString> PathElements;
	PathName.ParseIntoArray(PathElements, TEXT("/"), true);

	bool bRuleViolated = false;

	for (int32 i = 0; i < PathElements.Num() - 1; ++i)
	{
		if (DisallowedFolderNames.Contains(PathElements[i]))
		{
			OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), FText::FormatOrdered(RecommendedAction, FText::FromString(PathElements[i]))));
			bRuleViolated = true;
		}
	}
	
	return !bRuleViolated;
}