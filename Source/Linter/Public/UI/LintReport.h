// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SScrollBox.h"

#include "LintReportAssetError.h"
#include "LintRule.h"


class SLintReport : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLintReport)
	{
	}

	SLATE_END_ARGS()

public:
	
	void Construct(const FArguments& Args);
	void Rebuild(const ULintRuleSet* SelectedLintRuleSet);
	TSharedRef<SWidget> GetViewButtonContent();

	const ULintRuleSet* LastUsedRuleSet = nullptr;

	TSharedPtr<STextBlock> ResultsTextBlockPtr;
	TArray<TSharedPtr<FLintRuleViolation>> RuleViolations;
	TSharedPtr<class SComboButton> ViewOptionsComboButton;
	TSharedPtr<SScrollBox> AssetDetailsScrollBoxPtr;
	TSharedPtr<SScrollBox> RuleDetailsScrollBoxPtr;
	FString JsonReport;
	FString HTMLReport;

	bool bHasRanReport = false;
	int32 NumErrors = 0;
	int32 NumWarnings = 0;

	
};