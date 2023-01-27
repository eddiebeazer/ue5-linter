// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Blueprint_Compiles.h"
#include "LintRuleSet.h"
#include "Sound/SoundWave.h"
#include "Engine/Blueprint.h"

ULintRule_Blueprint_Compiles::ULintRule_Blueprint_Compiles(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Blueprint_Compiles::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	UBlueprint* Blueprint = CastChecked<UBlueprint>(ObjectToLint);

	switch (Blueprint->Status)
	{
	case EBlueprintStatus::BS_BeingCreated:
	case EBlueprintStatus::BS_Dirty:
	case EBlueprintStatus::BS_Unknown:
	case EBlueprintStatus::BS_UpToDate:
		return true;
	case EBlueprintStatus::BS_Error:
	case EBlueprintStatus::BS_UpToDateWithWarnings:
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass()));
		return false;
	default:
		return true;
	}
}