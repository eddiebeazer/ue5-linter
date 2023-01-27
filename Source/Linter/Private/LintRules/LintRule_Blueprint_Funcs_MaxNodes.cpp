// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Funcs_MaxNodes.h"
#include "LintRuleSet.h"
#include "Engine/Blueprint.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_Knot.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_Self.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_BreakStruct.h"
#include "EdGraphNode_Comment.h"
#include "K2Node_VariableGet.h"
#include "K2Node_StructMemberGet.h"
#include "K2Node_Tunnel.h"
#include "K2Node_TemporaryVariable.h"
#include "K2Node_FunctionTerminator.h"
#include "K2Node_CastByteToEnum.h"
#include "K2Node_CallFunction.h"

ULintRule_Blueprint_Funcs_MaxNodes::ULintRule_Blueprint_Funcs_MaxNodes(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Blueprint_Funcs_MaxNodes::IsNodeTrivial(const UEdGraphNode* Node)
{
	if (Node->IsA(UK2Node_Knot::StaticClass())
		|| Node->IsA(UK2Node_FunctionEntry::StaticClass())
		|| Node->IsA(UK2Node_Self::StaticClass())
		|| Node->IsA(UK2Node_DynamicCast::StaticClass())
		|| Node->IsA(UK2Node_BreakStruct::StaticClass())
		|| Node->IsA(UEdGraphNode_Comment::StaticClass())
		|| Node->IsA(UK2Node_VariableGet::StaticClass())
		|| Node->IsA(UK2Node_StructMemberGet::StaticClass())
		|| Node->IsA(UK2Node_Tunnel::StaticClass())
		|| Node->IsA(UK2Node_TemporaryVariable::StaticClass())
		|| Node->IsA(UK2Node_FunctionResult::StaticClass())
		|| Node->IsA(UK2Node_FunctionTerminator::StaticClass())
		|| Node->IsA(UK2Node_CastByteToEnum::StaticClass())
		)
	{
		return true;
	}

	if (const UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(Node))
	{
		FName FuncName = CallFuncNode->FunctionReference.GetMemberName();
		if (FuncName == TEXT("Conv_InterfaceToObject")
			|| FuncName.ToString().Contains(TEXT("MakeLiteral"))
			|| FuncName.ToString().Contains(TEXT("ToString"))
			|| FuncName.ToString().StartsWith(TEXT("Make"))
			|| FuncName.ToString().StartsWith(TEXT("Break"))
			)
		{
			return true;
		}
	}

	return false;
}

bool ULintRule_Blueprint_Funcs_MaxNodes::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	bool bRuleViolated = false;
	FText FixTextTemplate = NSLOCTEXT("Linter", "BlueprintFuncsMaxNodes", "{Previous}{WhiteSpace}Please simply function {FuncName} as it has {Nodes} nodes when we want a max of {MaxNodes}.");
	FText AllFixes;

	for (auto FunctionGraph : Blueprint->FunctionGraphs)
	{
		if (FunctionGraph->GetFName() != UEdGraphSchema_K2::FN_UserConstructionScript)
		{
			// If initial graph check exceeds node limit, filter out nodes that do not contribute to complexity
			if (FunctionGraph->Nodes.Num() > MaxExpectedNonTrivialNodes)
			{
				auto NodesCopy = FunctionGraph->Nodes;
				NodesCopy.RemoveAll([this](UEdGraphNode* Val) { return IsNodeTrivial(Val); });

				// If removing knots and comments still exceeds node limit, report error
				if (NodesCopy.Num() > MaxExpectedNonTrivialNodes)
				{
					AllFixes = FText::FormatNamed(FixTextTemplate, TEXT("Previous"), AllFixes, TEXT("FuncName"), FText::FromString(FunctionGraph->GetName()), TEXT("Nodes"), FText::FromString(FString::FromInt(NodesCopy.Num())), TEXT("MaxNodes"), FText::FromString(FString::FromInt(MaxExpectedNonTrivialNodes)), TEXT("WhiteSpace"), bRuleViolated ? FText::FromString(TEXT("\r\n")) : FText::GetEmpty());
					bRuleViolated = true;
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