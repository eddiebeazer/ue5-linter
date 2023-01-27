// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Funcs_PublicDescriptions.h"
#include "LintRuleSet.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_FunctionEntry.h"


ULintRule_Blueprint_Funcs_PublicDescriptions::ULintRule_Blueprint_Funcs_PublicDescriptions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Blueprint_Funcs_PublicDescriptions::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	// Early return out if blueprint type shouldn't be checked for function descriptions
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
	FText FixTextTemplate = NSLOCTEXT("Linter", "BlueprintFuncsPublicDescriptions", "{Previous}{WhiteSpace}Please give public function {FuncName} a description.");
	FText AllFixes;

	for (UEdGraph* FunctionGraph : Blueprint->FunctionGraphs)
	{
		if (FunctionGraph->GetFName() != UEdGraphSchema_K2::FN_UserConstructionScript)
		{
			UK2Node_FunctionEntry* FunctionEntryNode = nullptr;
			TArray<UK2Node_FunctionEntry*> EntryNodes;

			FunctionGraph->GetNodesOfClass(EntryNodes);

			if ((EntryNodes.Num() > 0) && EntryNodes[0]->IsEditable())
			{
				FunctionEntryNode = Cast<UK2Node_FunctionEntry>(EntryNodes[0]);
			}

			if (FunctionEntryNode != nullptr)
			{
				if (FUNC_AccessSpecifiers & FunctionEntryNode->GetFunctionFlags() & FUNC_Public)
				{
					if (FunctionEntryNode->MetaData.ToolTip.IsEmpty())
					{
						AllFixes = FText::FormatNamed(FixTextTemplate, TEXT("Previous"), AllFixes, TEXT("FuncName"), FText::FromString(FunctionGraph->GetName()), TEXT("WhiteSpace"), bRuleViolated ? FText::FromString(TEXT("\r\n")) : FText::GetEmpty());
						bRuleViolated = true;
					}
				}
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