// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_LooseNodes.h"
#include "LintRuleSet.h"
#include "Sound/SoundWave.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphNode_Comment.h"
#include "K2Node_Event.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_Knot.h"
#include "K2Node_Tunnel.h"

ULintRule_Blueprint_LooseNodes::ULintRule_Blueprint_LooseNodes(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Blueprint_LooseNodes::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	// Check for loose nodes
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);

	for (UEdGraph* Graph : Graphs)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (
				Node->IsAutomaticallyPlacedGhostNode() ||
				Node->IsA(UK2Node_Event::StaticClass()) ||
				Node->IsA(UK2Node_FunctionEntry::StaticClass()) ||
				Node->IsA(UK2Node_Knot::StaticClass()) ||
				Node->IsA(UEdGraphNode_Comment::StaticClass()) ||
				Node->IsA(UK2Node_Tunnel::StaticClass())
				)
			{
				continue;
			}

			bool bNodeIsolated = true;

			TArray<UEdGraphPin*> Pins = Node->GetAllPins();
			for (UEdGraphPin* Pin : Pins)
			{
				if (Pin->LinkedTo.Num() != 0)
				{
					bNodeIsolated = false;
					break;
				}
			}

			if (bNodeIsolated)
			{
				OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass()));
				return false;
			}
		}
	}

	return true;

}