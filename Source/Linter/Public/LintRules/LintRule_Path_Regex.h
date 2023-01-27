// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.h"

#include "LintRule_Path_Regex.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule_Path_Regex : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_Path_Regex(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category="Settings")
	FString RegexPatternString;

	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bMustNotContainRegexPattern = true;

	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bCheckPerPathElement = true;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Whole Path")
	FText DisallowedWholePathRecommendedAction;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Whole Path")
	FText NonConformingWholePathRecommendedAction;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Path Element")
	FText DisallowedPathElementRecommendedAction;

	UPROPERTY(EditDefaultsOnly, Category = "Settings|Path Element")
	FText NonConformingPathElementRecommendedAction;

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

};
