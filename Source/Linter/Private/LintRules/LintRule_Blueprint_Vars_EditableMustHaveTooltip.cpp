// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Vars_EditableMustHaveTooltip.h"
#include "LintRuleSet.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"

ULintRule_Blueprint_Vars_EditableMustHaveTooltip::ULintRule_Blueprint_Vars_EditableMustHaveTooltip(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Blueprint_Vars_EditableMustHaveTooltip::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	bool bRuleViolated = false;

	FText FixTextTemplate = NSLOCTEXT("Linter", "BlueprintVarsEditableMustHaveTooltip", "{Previous}{WhiteSpace}Please give variable {VarName} a tooltip as it is marked editable.");
	FText AllFixes;
	
	for (FBPVariableDescription Desc : Blueprint->NewVariables)
	{
		FString PropName = Desc.VarName.ToString();
		FText TypeName = UEdGraphSchema_K2::TypeToText(Desc.VarType);

		if ((Desc.PropertyFlags & CPF_DisableEditOnInstance) != CPF_DisableEditOnInstance)
		{
			if (!Desc.HasMetaData(FBlueprintMetadata::MD_Tooltip) || Desc.GetMetaData(FBlueprintMetadata::MD_Tooltip).Len() <= 0)
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