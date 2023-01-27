// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Collection.h"
#include "LintRuleSet.h"
#include "LinterNamingConvention.h"
#include "HAL/FileManager.h"

ULintRule_Collection::ULintRule_Collection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Collection::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	bool bRuleViolated = false;

	for (TSubclassOf<ULintRule> LintRuleClass : SubRules)
	{
		if (LintRuleClass.Get() != nullptr)
		{
			const ULintRule* LintRule = GetDefault<ULintRule>(LintRuleClass);
			if (LintRule != nullptr)
			{
				TArray<FLintRuleViolation> SubViolations;
				if (!LintRule->PassesRule(ObjectToLint, ParentRuleSet, SubViolations))
				{
					OutRuleViolations.Append(SubViolations);
					bRuleViolated = true;
				}
			}
		}
	}

	return !bRuleViolated;
}