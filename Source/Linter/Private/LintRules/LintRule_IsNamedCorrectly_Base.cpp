// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_IsNamedCorrectly_Base.h"
#include "LintRuleSet.h"
#include "LinterNamingConvention.h"

ULintRule_IsNamedCorrectly_Base::ULintRule_IsNamedCorrectly_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

bool ULintRule_IsNamedCorrectly_Base::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	// NameSettingList Contributed by RocknRolla#3102 on http://discord.gamemak.in
	TArray<FLinterNamingConventionInfo> NameSettingList;
	// If ObjectToLint is a Blueprint, it's class hierarchy does not contain actual classes that Blueprints extend (like Actor, Character, PlayerController, etc.).
	// So we try to use Blueprint's ParentClass to access it's class hierarchy.
	if (ObjectToLint->IsA<UBlueprint>())
	{
		const TSubclassOf<UObject> BlueprintClass = Cast<UBlueprint>(ObjectToLint)->ParentClass;
		if (BlueprintClass != nullptr)
		{
			NameSettingList = ParentRuleSet->GetNamingConvention()->GetNamingConventionsForClassVariant(TSoftClassPtr<UObject>(BlueprintClass), GetRuleBasedObjectVariantName(ObjectToLint));
		}
	}
	// If ObjectToLint is not a Blueprint or we failed to find any conventions, just fall back to our default algorithm.
	if (NameSettingList.Num() == 0)
	{
		NameSettingList = ParentRuleSet->GetNamingConvention()->GetNamingConventionsForClassVariant(ObjectToLint->GetClass(), GetRuleBasedObjectVariantName(ObjectToLint));
	}

	// If we don't have a name rule for this type of asset, simply return true	
	if (NameSettingList.Num() == 0)
	{
		return true;
	}

	bool bFoundMatchingNameRule = false;
	for (FLinterNamingConventionInfo Info : NameSettingList)
	{
		bool bPassesPrefixCheck = Info.Prefix.IsEmpty() ? true : ObjectToLint->GetName().StartsWith(Info.Prefix, ESearchCase::CaseSensitive);
		bool bPassesSuffixCheck = Info.Suffix.IsEmpty() ? true : ObjectToLint->GetName().EndsWith(Info.Suffix, ESearchCase::CaseSensitive);
		// Run prefix and suffix checks using found name settings if they are non-null

		if (bPassesPrefixCheck && bPassesSuffixCheck)
		{
			bFoundMatchingNameRule = true;
			break;
		}
	}

	if (!bFoundMatchingNameRule)
	{
		FString SuggestedName = BuildSuggestedName(ObjectToLint->GetName(), NameSettingList[0].Prefix, NameSettingList[0].Suffix);
		FText RecommendedAction = FText::FormatOrdered(NSLOCTEXT("Linter", "IsNamedCorrectly_RecommendedAction", "Recommended name: [{0}]."), FText::FromString(SuggestedName));
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass(), RecommendedAction));
		return false;
	}

	// If we don't have name settings or passed all name checks, simply return true
	return true;
}

FString ULintRule_IsNamedCorrectly_Base::BuildSuggestedName(FString CurrentName, FString DesiredPrefix, FString DesiredSuffix /*= TEXT("")*/)
{
	FString SuggestedName;

	int32 FirstUnderscore = CurrentName.Find(TEXT("_"));
	int32 LastUnderscore = CurrentName.Find(TEXT("_"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	bool bAddPrefix = false;
	bool bAddSuffix = false;

	// Attempt to remove a bad prefix
	if (!DesiredPrefix.IsEmpty() && !CurrentName.StartsWith(DesiredPrefix, ESearchCase::CaseSensitive))
	{
		bAddPrefix = true;
		if (FirstUnderscore <= 3)
		{
			CurrentName = CurrentName.RightChop(FirstUnderscore + 1);
		}
	}

	// Attempt to remove a bad suffix
	if (!DesiredSuffix.IsEmpty() && !CurrentName.EndsWith(DesiredSuffix, ESearchCase::CaseSensitive))
	{
		bAddSuffix = true;
		if (CurrentName.Len() - LastUnderscore <= 3)
		{
			CurrentName = CurrentName.LeftChop(CurrentName.Len() - LastUnderscore);
		}
	}

	SuggestedName = CurrentName;

	if (bAddPrefix)
	{
		SuggestedName = DesiredPrefix + SuggestedName;
	}

	if (bAddSuffix)
	{
		SuggestedName = SuggestedName + DesiredSuffix;
	}

	return SuggestedName;
}
