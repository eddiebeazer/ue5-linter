// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Vars_ConfigCategories.h"
#include "LintRuleSet.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"

ULintRule_Blueprint_Vars_ConfigCategories::ULintRule_Blueprint_Vars_ConfigCategories(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Blueprint_Vars_ConfigCategories::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	bool bRuleViolated = false;

	FText FixTextTemplate = NSLOCTEXT("Linter", "BlueprintVarsConfigCategories", "{Previous}{WhiteSpace}Please give variable {VarName} a category.");
	FText FixTextTemplateEditable = NSLOCTEXT("Linter", "BlueprintVarsConfigCategoriesEditable", "{Previous}{WhiteSpace}Please give editable variable {VarName} a category starting with 'Config'.");
	FText AllFixes;

	int32 VariableCount = Blueprint->NewVariables.Num();
	for (FBPVariableDescription Desc : Blueprint->NewVariables)
	{
		if (FBlueprintEditorUtils::IsVariableComponent(Desc))
		{
			VariableCount--;
		}
	}

	if (VariableCount < NumVariablesToRequireCategorization)
	{
		return true;
	}	
	
	for (FBPVariableDescription Desc : Blueprint->NewVariables)
	{
		FString PropName = Desc.VarName.ToString();
		FText TypeName = UEdGraphSchema_K2::TypeToText(Desc.VarType);

		// Is Editable variable?
		if ((Desc.PropertyFlags & CPF_DisableEditOnInstance) != CPF_DisableEditOnInstance)
		{
			if (!Desc.Category.ToString().StartsWith(TEXT("Config")))
			{
				AllFixes = FText::FormatNamed(FixTextTemplateEditable, TEXT("Previous"), AllFixes, TEXT("VarName"), FText::FromString(PropName), TEXT("WhiteSpace"), bRuleViolated ? FText::FromString(TEXT("\r\n")) : FText::GetEmpty());
				bRuleViolated = true;
				continue;
			}
		}
		else
		{
			if (Desc.Category.IsEmptyOrWhitespace())
			{
				AllFixes = FText::FormatNamed(FixTextTemplate, TEXT("Previous"), AllFixes, TEXT("VarName"), FText::FromString(PropName), TEXT("WhiteSpace"), bRuleViolated ? FText::FromString(TEXT("\r\n")) : FText::GetEmpty());
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