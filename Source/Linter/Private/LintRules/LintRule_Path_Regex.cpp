// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Path_Regex.h"
#include "LintRuleSet.h"
#include "LinterNamingConvention.h"
#include "HAL/FileManager.h"
#include "Internationalization/Regex.h"

ULintRule_Path_Regex::ULintRule_Path_Regex(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RegexPatternString(TEXT("[^a-zA-Z0-9_]"))
{
	DisallowedPathElementRecommendedAction = NSLOCTEXT("Linter", "ULintRule_Path_Regex_DisallowedPathElement", "Please rename \"{0}\" and remove disallowed characters.");
	NonConformingPathElementRecommendedAction = NSLOCTEXT("Linter", "ULintRule_Path_Regex_NonConformingPathElement", "Please rename \"{0}\" and to conform to allowed characters.");

	DisallowedWholePathRecommendedAction = NSLOCTEXT("Linter", "ULintRule_Path_Regex_DisallowedWholePath", "Please rename and remove disallowed characters.");
	NonConformingWholePathRecommendedAction = NSLOCTEXT("Linter", "ULintRule_Path_Regex_NonConformingWholePath", "Please rename and conform to allowed characters.");
}

bool ULintRule_Path_Regex::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	FString PathName = ObjectToLint->GetPathName();
	
	FRegexPattern RegexPattern = FRegexPattern(RegexPatternString);
	bool bRuleViolated = false;

	if (bCheckPerPathElement)
	{
		TArray<FString> PathElements;
		PathName.ParseIntoArray(PathElements, TEXT("/"), true);

		for (int32 i = 0; i < PathElements.Num() - 1; ++i)
		{
			FRegexMatcher RegexMatcher(RegexPattern, PathElements[i]);
			bool bFoundMatch = RegexMatcher.FindNext();

			if ((bFoundMatch && bMustNotContainRegexPattern) || (!bFoundMatch && !bMustNotContainRegexPattern))
			{
				OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), FText::FormatOrdered(bMustNotContainRegexPattern ? DisallowedPathElementRecommendedAction : NonConformingPathElementRecommendedAction, FText::FromString(PathElements[i]))));
				bRuleViolated = true;
			}
		}
	}
	else
	{
		FRegexMatcher RegexMatcher(RegexPattern, PathName);
		bool bFoundMatch = RegexMatcher.FindNext();

		if ((bFoundMatch && bMustNotContainRegexPattern) || (!bFoundMatch && !bMustNotContainRegexPattern))
		{
			OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), FText::FormatOrdered(bMustNotContainRegexPattern ? DisallowedWholePathRecommendedAction : NonConformingWholePathRecommendedAction, FText::FromString(PathName))));
			bRuleViolated = true;
		}
	}

	return !bRuleViolated;
}