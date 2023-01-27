// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.h"

#include "LintRule_Collection.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule_Collection : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_Collection(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TArray<TSubclassOf<ULintRule>> SubRules;

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

};
