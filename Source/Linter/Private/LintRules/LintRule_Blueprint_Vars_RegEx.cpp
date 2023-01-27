// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Vars_Regex.h"
#include "LintRuleSet.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"
#include "Internationalization/Regex.h"

ULintRule_Blueprint_Vars_Regex::ULintRule_Blueprint_Vars_Regex(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


bool ULintRule_Blueprint_Vars_Regex::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	bool bRuleViolated = false;

	FText FixTextTemplate = NSLOCTEXT("Linter", "BlueprintVarsRegex", "{Previous}{WhiteSpace}Please fix variable named {VarName}.");
	FText AllFixes;

	FString TestRegexPatternString = RegexPatternString;
	FString BoolTestRegexPatternString = TEXT("b") + RegexPatternString;

	const FRegexPattern TestRegexPattern = FRegexPattern(TestRegexPatternString);
	const FRegexPattern BoolTestRegexPattern = FRegexPattern(BoolTestRegexPatternString);

	for (FBPVariableDescription Desc : Blueprint->NewVariables)
	{
		FString PropName = Desc.VarName.ToString();
		FText TypeName = UEdGraphSchema_K2::TypeToText(Desc.VarType);
		bool bIsBool = Desc.VarType.PinCategory == UEdGraphSchema_K2::PC_Boolean;
		
		FRegexMatcher Matcher(bIsBool ? BoolTestRegexPattern : TestRegexPattern, PropName);
		bool bFoundMatch = Matcher.FindNext();

		if ((bFoundMatch && bMustNotContainRegexPattern) || (!bFoundMatch && !bMustNotContainRegexPattern))
		{
			AllFixes = FText::FormatNamed(FixTextTemplate, TEXT("Previous"), AllFixes, TEXT("VarName"), FText::FromString(PropName), TEXT("WhiteSpace"), bRuleViolated ? FText::FromString(TEXT("\r\n")) : FText::GetEmpty());
			bRuleViolated = true;
		}
	}

	if (bRuleViolated)
	{
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), AllFixes));
		return false;
	}

	return true;
}