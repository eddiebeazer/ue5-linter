// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_ParticleSystem_EmitterNameRegex.h"
#include "LintRuleSet.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleSystem.h"
#include "Internationalization/Regex.h"

ULintRule_ParticleSystem_EmitterNameRegex::ULintRule_ParticleSystem_EmitterNameRegex(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RegexPatternString(TEXT("Particle Emitter"))
{
	DisallowedRecommendedAction = NSLOCTEXT("Linter", "ULintRule_ParticleSystem_EmitterRegexName_Disallowed", "Please rename the emitter \"{0}\" as you have multiple emitters.");
	NonConformingRecommendedAction = NSLOCTEXT("Linter", "ULintRule_ParticleSystem_EmitterRegexName_NonConforming", "Please rename \"{0}\" as this emitter has invalid characters.");
}

bool ULintRule_ParticleSystem_EmitterNameRegex::PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	// If we aren't a particle system, abort
	if (Cast<UParticleSystem>(ObjectToLint) == nullptr)
	{
		// @TODO: Bubble up some sort of configuration error?
		return true;
	}

	return Super::PassesRule(ObjectToLint, ParentRuleSet, OutRuleViolations);
}

bool ULintRule_ParticleSystem_EmitterNameRegex::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	const UParticleSystem* ParticleSystem = CastChecked<UParticleSystem>(ObjectToLint);

	bool bRuleViolated = false;

	FText FixTextTemplate = NSLOCTEXT("Linter", "ParticleHasBadEmitterNames", "{Previous}{WhiteSpace}Please rename emitter {EmitterName}.");
	FText AllFixes;

	if (ParticleSystem->Emitters.Num() >= MinEmittersNeededToEnforce)
	{
		FRegexPattern RegexPattern = FRegexPattern(RegexPatternString);
		
		for (UParticleEmitter* Emitter : ParticleSystem->Emitters)
		{
			FRegexMatcher RegexMatcher(RegexPattern, Emitter->EmitterName.ToString());
			bool bFoundMatch = RegexMatcher.FindNext();

			if ((bFoundMatch && bMustNotContainRegexPattern) || (!bFoundMatch && !bMustNotContainRegexPattern))
			{
				AllFixes = FText::FormatNamed(FixTextTemplate, TEXT("Previous"), AllFixes, TEXT("EmitterName"), FText::FromString(Emitter->EmitterName.ToString()), TEXT("WhiteSpace"), bRuleViolated ? FText::FromString(TEXT("\r\n")) : FText::GetEmpty());
				bRuleViolated = true;
			}
		}
	}

	if (bRuleViolated)
	{
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), AllFixes));
		return false;
	}

	return true;
}