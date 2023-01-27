// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Base.h"
#include "LintRuleSet.h"
#include "Engine/Blueprint.h"

ULintRule_Blueprint_Base::ULintRule_Blueprint_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Blueprint_Base::PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	// If we aren't a blueprint, abort
	if (Cast<UBlueprint>(ObjectToLint) == nullptr)
	{
		// @TODO: Bubble up some sort of configuration error?
		return true;
	}

	return Super::PassesRule(ObjectToLint, ParentRuleSet, OutRuleViolations);
}