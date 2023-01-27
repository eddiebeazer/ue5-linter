// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.h"

#include "LintRule_Blueprint_Base.generated.h"

/**
 *Comment
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule_Blueprint_Base : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_Blueprint_Base(const FObjectInitializer& ObjectInitializer);

	// This does rule pre-checks. You probably want to override PassesRule_Internal_Implementation
	virtual bool PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;
};
