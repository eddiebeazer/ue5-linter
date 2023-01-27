// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Funcs_MustHaveReturn.h"
#include "LintRuleSet.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_FunctionResult.h"

ULintRule_Blueprint_Funcs_MustHaveReturn::ULintRule_Blueprint_Funcs_MustHaveReturn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Blueprint_Funcs_MustHaveReturn::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	// Early return out if blueprint type shouldn't be checked for return nodes
	switch (Blueprint->BlueprintType)
	{
	case BPTYPE_Normal:
	case BPTYPE_Const:
	case BPTYPE_LevelScript:
	case BPTYPE_FunctionLibrary:
		break;
	case BPTYPE_MacroLibrary:
	case BPTYPE_Interface:
	default:
		return true;
	}

	bool bRuleViolated = false;
	FText FixTextTemplate = NSLOCTEXT("Linter", "BlueprintFuncsMustHaveReturn", "{Previous}{WhiteSpace}Please give function {FuncName} a return node.");
	FText AllFixes;

	static const FName DefaultAnimGraphName("AnimGraph");

	for (auto FunctionGraph : Blueprint->FunctionGraphs)
	{
		if (FunctionGraph->GetFName() != UEdGraphSchema_K2::FN_UserConstructionScript
			&& FunctionGraph->GetFName() != DefaultAnimGraphName)
		{
			TArray<UK2Node_FunctionResult*> AllResultNodes;
			FunctionGraph->GetNodesOfClass(AllResultNodes);
			if (AllResultNodes.Num() <= 0)
			{
				AllFixes = FText::FormatNamed(FixTextTemplate, TEXT("Previous"), AllFixes, TEXT("FuncName"), FText::FromString(FunctionGraph->GetName()), TEXT("WhiteSpace"), bRuleViolated ? FText::FromString(TEXT("\r\n")) : FText::GetEmpty());
				bRuleViolated = true;
			}
		}
	}

	if (bRuleViolated)
	{
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), AllFixes));
		return false;
	}

	return true;
}