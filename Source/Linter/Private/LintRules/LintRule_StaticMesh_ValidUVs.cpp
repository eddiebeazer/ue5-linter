// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_StaticMesh_ValidUVs.h"
#include "LintRuleSet.h"
#include "LinterNamingConvention.h"
#include "HAL/FileManager.h"

ULintRule_StaticMesh_ValidUVs::ULintRule_StaticMesh_ValidUVs(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//UStaticMesh::CheckLightMapUVs requires being ran on the game thread
	bRequiresGameThread = true;
}

bool ULintRule_StaticMesh_ValidUVs::PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	// If we aren't a static mesh, abort
	if (Cast<UStaticMesh>(ObjectToLint) == nullptr)
	{
		// @TODO: Bubble up some sort of configuration error?
		return true;
	}

	return Super::PassesRule(ObjectToLint, ParentRuleSet, OutRuleViolations);	
}

bool ULintRule_StaticMesh_ValidUVs::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	const UStaticMesh* StaticMesh = CastChecked<UStaticMesh>(ObjectToLint);

	bool bHadErrors = false;

	TArray<FString> MissingUVs;
	TArray<FString> BadUVs;
	TArray<FString> ValidUVs;

	UStaticMesh::CheckLightMapUVs(const_cast<UStaticMesh*>(StaticMesh), MissingUVs, BadUVs, ValidUVs, true);

	if ((!bIgnoreMissingUVs && MissingUVs.Num() > 0))
	{
		FText RecommendedAction = NSLOCTEXT("Linter", "LintRule_StaticMesh_ValidUVs_Missing", "Static mesh has missing UVs. Please add at least one valid UV channel.");
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), RecommendedAction));
		return false;
	}

	if (BadUVs.Num() > 0)
	{
		FText RecommendedAction = NSLOCTEXT("Linter", "LintRule_StaticMesh_ValidUVs_Bad", "Static mesh has invalid UVs. [{0}]");
		FText::FormatOrdered(RecommendedAction, FText::FromString(FString::Join(BadUVs, TEXT(", "))));
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), RecommendedAction));
		return false;
	}

	return true;
}