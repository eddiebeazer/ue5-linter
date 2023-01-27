// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once
#include "Widgets/SCompoundWidget.h"
#include "LintRule.h"

class SLintReportAssetDetails : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLintReportAssetDetails)
	{
	}
	SLATE_ATTRIBUTE(FAssetData, AssetData)
	SLATE_ATTRIBUTE(TArray<TSharedPtr<FLintRuleViolation>>, RuleViolations)
	SLATE_ATTRIBUTE(TSharedPtr<FAssetThumbnailPool>, ThumbnailPool)
	
	SLATE_END_ARGS()

	TAttribute<FAssetData> AssetData;
	TAttribute<TArray<TSharedPtr<FLintRuleViolation>>> RuleViolations;
	TAttribute<TSharedPtr<FAssetThumbnailPool>> ThumbnailPool;


public:
	
	void Construct(const FArguments& Args);
};