// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.h"

#include "LintRule_SoundWave_SampleRate.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule_SoundWave_SampleRate : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_SoundWave_SampleRate(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TArray<float> ValidSampleRates;


	virtual bool PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

};
