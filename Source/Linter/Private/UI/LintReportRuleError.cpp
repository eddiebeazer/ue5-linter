// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "UI/LintReportRuleError.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Input/SHyperlink.h"
#include "LintRule.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "LintReport"


void SLintReportRuleError::Construct(const FArguments& Args)
{
	RuleViolation = Args._RuleViolation;
	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(PaddingAmount)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(PaddingAmount)
			.AutoWidth()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(SHyperlink)
				.Text(FText::FromName(RuleViolation.Get()->ViolatorAssetData.PackageName))
				.OnNavigate_Lambda([&]()
				{
					FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
					FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
					TArray<FAssetData> AssetDatas;
					AssetDatas.Push(AssetRegistryModule.Get().GetAssetByObjectPath(RuleViolation.Get()->ViolatorAssetData.ObjectPath));
					ContentBrowserModule.Get().SyncBrowserToAssets(AssetDatas);
				})
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.Padding(20.0f, PaddingAmount, PaddingAmount, PaddingAmount)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(RuleViolation.Get()->RecommendedAction)
			.Visibility(RuleViolation.Get()->RecommendedAction.IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible)
		]
	];
}

