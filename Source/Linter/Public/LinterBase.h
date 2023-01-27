// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#pragma once
#include "Internationalization/Regex.h"
#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "UObject/Object.h"

/** Future Notes:
 * The data structure for Linter was originally pretty straight forward. Now that we are introducing
 * some 'smart' behavior to some of the errors i.e. arbitrary automated resolve actions
 * and also tabulating data in both asset first and rule first orders, instead of being a
 * set of loosely defined rules, rules should ideally become a strongly typed const table of some kind.
 * This would simplify a lot of the data and structs being used here. -- Allar
**/

struct LINTER_API FLinterAssetError
{
	/** User friendly error message ready for displaying. */
	FText ErrorMessage;

	/** URL Link to link user to if they want more information on this error. */
	FString URLLink;

	FLinterAssetError(FText InErrorMessage, FString InURLLink = TEXT(""))
		: ErrorMessage(InErrorMessage)
		, URLLink(InURLLink)
	{
	}
};

struct LINTER_API FLinterAssetErrorList
{
	FString AssetName;
	FString AssetPath;
	FString SuggestedAssetName;
	TArray<TSharedPtr<FLinterAssetError>> Errors;
	TArray<TSharedPtr<FLinterAssetError>> Warnings;


	FLinterAssetErrorList() {};

	FLinterAssetErrorList(FString InAssetName, FString InAssetPath, TArray<TSharedPtr<FLinterAssetError>> InErrors, TArray<TSharedPtr<FLinterAssetError>> InWarnings, FString InSuggestedAssetName = TEXT(""))
		: AssetName(InAssetName)
		, AssetPath(InAssetPath)
        , SuggestedAssetName(InSuggestedAssetName)
		, Errors(InErrors)
		, Warnings(InWarnings)
	{
	}

	FLinterAssetErrorList(const UObject* Object, TArray<TSharedPtr<FLinterAssetError>> InErrors, TArray<TSharedPtr<FLinterAssetError>> InWarnings, FString InSuggestedAssetName = TEXT(""))
		: AssetName(Object->GetName())
		, AssetPath(Object->GetPathName())
        , SuggestedAssetName(InSuggestedAssetName)
		, Errors(InErrors)
		, Warnings(InWarnings)
		
	{
	}

	void Reset()
	{
		AssetName.Empty();
		AssetPath.Empty();
		SuggestedAssetName.Empty();
		Errors.Empty();
		Warnings.Empty();
	}

	bool IsEmpty()
	{
		return Errors.Num() == 0 && Warnings.Num() == 0;
	}

	bool HasErrors()
	{
		return Errors.Num() != 0;
	}
};

/* Linter was originally built to store its data on a per-asset basis. 
 * After Epic's purchase, UI needed a way to store data on a per-rule basis.
 * The following structs help out the LinterManager populate a per-rule list after Linting is complete. */

struct LINTER_API FLinterAssetInfo
{
	FString AssetName;
	FString AssetPath;
	FString SuggestedAssetName;

	FText RuleErrorContext;

	FLinterAssetInfo(const UObject* Object, FText InRuleErrorContext = FText::GetEmpty(), FString InSuggestedAssetName = TEXT(""))
	{
		AssetName = Object->GetName();
		AssetPath = Object->GetPathName();
		SuggestedAssetName = InSuggestedAssetName;
	}
};

struct LINTER_API FLinterRuleErrorList
{
	FText RuleMessage;
	FString RuleURL;
	bool bWarning;
	TArray<TSharedPtr<FLinterAssetInfo>> AssetInfos;

	FLinterRuleErrorList()
		: bWarning(false)
	{
	}

	FLinterRuleErrorList(FText InRuleMessage, FText InRuleContext, bool bInWarning, const UObject* Object, FString InRuleURL, FString InSuggestedAssetName = TEXT(""))
		: RuleMessage(InRuleMessage)
		, RuleURL(InRuleURL)
		, bWarning(bInWarning)
	{
		AssetInfos.Add(MakeShareable(new FLinterAssetInfo(Object, InRuleContext, InSuggestedAssetName)));
	}
};