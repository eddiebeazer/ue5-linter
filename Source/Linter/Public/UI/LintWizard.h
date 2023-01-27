// Copyright 2015-2017 by Gamemakin LLC

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Workflow/SWizard.h"
#include "UI/SStepWidget.h"

#include "LintReport.h"

/**
 * 
 */
class LINTER_API SLintWizard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLintWizard)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	/** The wizard widget */
	TSharedPtr<SWizard> MainWizard;

	/** The Lint Report widget */
	TSharedPtr<SLintReport> LintReport;

	/** The Linter combo box selection to use to select a linter rule set. */
	TSharedPtr<SComboBox<TSharedPtr<FAssetData>>> RuleSetSelectionComboBox;

	/** List of Linter managers grabbed from the Linter Module on widget creation */
	TArray<TSharedPtr<FAssetData>> RuleSets;

	/** List of maps in the project used for wizard purposes */
	TArray<TSharedPtr<FAssetData>> MapAssetDataList;

	/** Scrollbox for list of map assets in marketplace recommendation page */
	TSharedPtr<SScrollBox> MarketplaceRecommendationMapScrollBoxPtr;

	/** Currently selected Linter Rule Set */
	TSharedPtr<FAssetData> SelectedRuleSet;

	bool bOfferPackage = false;
	EStepStatus FixUpRedirectorStatus = EStepStatus::Unknown;
	EStepStatus SaveAllStatus = EStepStatus::Unknown;

	void OnLintReportEntered();
	void OnMarketplaceRecommendationsEntered();

	bool LoadAssetsIfNeeded(const TArray<FString>& ObjectPaths, TArray<UObject*>& LoadedObjects);
};
