// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Vars_PluralArrays.h"
#include "LintRuleSet.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"

ULintRule_Blueprint_Vars_PluralArrays::ULintRule_Blueprint_Vars_PluralArrays(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Blueprint_Vars_PluralArrays::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	bool bRuleViolated = false;

	FText FixTextTemplate = NSLOCTEXT("Linter", "PluralArrayHasArray", "{Previous}{WhiteSpace}Please remove the word 'Array' from your variable {VarName}.");
	FText AllFixes;
	
	for (FBPVariableDescription Desc : Blueprint->NewVariables)
	{
		FString PropName = Desc.VarName.ToString();
		FText TypeName = UEdGraphSchema_K2::TypeToText(Desc.VarType);

		if (PropName.Contains(TEXT("Array"), ESearchCase::CaseSensitive))
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