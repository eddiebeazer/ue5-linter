// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.h"

#include "LintRule_ParticleSystem_EmitterNameRegex.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule_ParticleSystem_EmitterNameRegex : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_ParticleSystem_EmitterNameRegex(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 MinEmittersNeededToEnforce = 2;

	UPROPERTY(EditAnywhere, Category="Settings")
	FString RegexPatternString;

	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bMustNotContainRegexPattern = true;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	FText DisallowedRecommendedAction;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	FText NonConformingRecommendedAction;


	virtual bool PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

};
