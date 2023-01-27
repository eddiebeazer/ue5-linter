// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Vars_NonAtomic.h"
#include "LintRuleSet.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"
#include "Internationalization/Regex.h"

ULintRule_Blueprint_Vars_NonAtomic::ULintRule_Blueprint_Vars_NonAtomic(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


bool ULintRule_Blueprint_Vars_NonAtomic::IsVariableAtomic(FBPVariableDescription& VarDesc)
{
	return (VarDesc.VarType.PinCategory == UEdGraphSchema_K2::PC_Boolean
		|| VarDesc.VarType.PinCategory == UEdGraphSchema_K2::PC_Byte
		|| VarDesc.VarType.PinCategory == UEdGraphSchema_K2::PC_Int
		|| VarDesc.VarType.PinCategory == UEdGraphSchema_K2::PC_Float
		|| VarDesc.VarType.PinCategory == UEdGraphSchema_K2::PC_String
		|| VarDesc.VarType.PinCategory == UEdGraphSchema_K2::PC_Enum
		);
}

bool ULintRule_Blueprint_Vars_NonAtomic::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	bool bRuleViolated = false;

	FText FixTextTemplate = NSLOCTEXT("Linter", "BlueprintVarsNonAtomic", "{Previous}{WhiteSpace}Please fix variable named {VarName}.");
	FText AllFixes;
	
	for (FBPVariableDescription Desc : Blueprint->NewVariables)
	{
		FString PropName = Desc.VarName.ToString();
		FText TypeName = UEdGraphSchema_K2::TypeToText(Desc.VarType);

		if (IsVariableAtomic(Desc) && PropName.Contains(TypeName.ToString()))
		{
			AllFixes = FText::FormatNamed(FixTextTemplate, TEXT("Previous"), AllFixes, TEXT("VarName"), FText::FromString(PropName), TEXT("WhiteSpace"), bRuleViolated ? FText::FromString(TEXT("\r\n")) : FText::GetEmpty());
			bRuleViolated = true;
		}
	}

	if (bRuleViolated)
	{
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass()));
		return false;
	}

	return true;
}