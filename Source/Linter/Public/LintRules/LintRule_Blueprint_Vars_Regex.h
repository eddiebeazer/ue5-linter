// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.h"

#include "LintRule_Blueprint_Vars_Regex.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule_Blueprint_Vars_Regex : public ULintRule_Blueprint_Base
{
	GENERATED_BODY()

public:
	ULintRule_Blueprint_Vars_Regex(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	bool bUseLowercaseBPrefixForBooleans = true;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FString RegexPatternString;

	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bMustNotContainRegexPattern = true;

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

};
