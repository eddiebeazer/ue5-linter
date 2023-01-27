// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_SoundWave_SampleRate.h"
#include "LintRuleSet.h"
#include "Sound/SoundWave.h"

ULintRule_SoundWave_SampleRate::ULintRule_SoundWave_SampleRate(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ValidSampleRates.Push(22050.0f);
	ValidSampleRates.Push(44100.0f);
}

bool ULintRule_SoundWave_SampleRate::PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	// If we aren't a sound wave, abort
	if (Cast<USoundWave>(ObjectToLint) == nullptr)
	{
		// @TODO: Bubble up some sort of configuration error?
		return true;
	}

	return Super::PassesRule(ObjectToLint, ParentRuleSet, OutRuleViolations);
}

bool ULintRule_SoundWave_SampleRate::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	USoundWave* SoundWave = const_cast<USoundWave*>(CastChecked<USoundWave>(ObjectToLint));

	if (ValidSampleRates.Contains(SoundWave->GetSampleRateForCurrentPlatform()))
	{
		return true;
	}

	FText RecommendedAction = NSLOCTEXT("Linter", "LintRule_SoundWave_SampleRate_Fix", "Please fix your sample rate of {0}.");
	RecommendedAction = FText::FormatOrdered(RecommendedAction, SoundWave->GetSampleRateForCurrentPlatform());

	OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), RecommendedAction));
	
	return true;
}