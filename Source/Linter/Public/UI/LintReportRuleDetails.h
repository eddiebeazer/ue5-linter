// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once
#include "Widgets/SCompoundWidget.h"
#include "LintRule.h"

class SLintReportRuleDetails : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLintReportRuleDetails)
	{
	}
	SLATE_ATTRIBUTE(TArray<TSharedPtr<FLintRuleViolation>>, RuleViolations)
	SLATE_ATTRIBUTE(TSharedPtr<FAssetThumbnailPool>, ThumbnailPool)
	
	SLATE_END_ARGS()

	TAttribute<TArray<TSharedPtr<FLintRuleViolation>>> RuleViolations;
	TAttribute<TSharedPtr<FAssetThumbnailPool>> ThumbnailPool;


public:
	
	void Construct(const FArguments& Args);

private:
	FString RuleURL;
	FAssetData RuleAssetData;
	TSharedPtr<SBox> ThumbnailBox;
};