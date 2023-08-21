// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "UI/LintReportAssetDetails.h"
#include "LinterStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "EditorStyleSet.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistryModule.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SSpacer.h"
#include "IAssetTools.h"
#include "AssetToolsModule.h"
#include "Misc/MessageDialog.h"
#include "Internationalization/Internationalization.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Views/ITypedTableView.h"
#include "UI/LintReportAssetError.h"
#include "LintRule.h"
#include "AssetThumbnail.h"
#include "UI/LintReportAssetErrorList.h"


#define LOCTEXT_NAMESPACE "LintReport"

void SLintReportAssetDetails::Construct(const FArguments& Args)
{
	AssetData = Args._AssetData;
	RuleViolations = Args._RuleViolations;
	ThumbnailPool = Args._ThumbnailPool;

	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

	FText AssetName = FText::FromString(AssetData.Get().AssetName.ToString());
	FText AssetPath = FText::FromString(AssetData.Get().GetFullName());

	const TSharedPtr<FAssetThumbnail> AssetThumbnail = MakeShareable(new FAssetThumbnail(AssetData.Get().GetAsset(), 96, 96, ThumbnailPool.Get()));

	int32 NumErrors = 0;
	int32 NumWarnings = 0;

	for (TSharedPtr<FLintRuleViolation> RuleViolation : RuleViolations.Get())
	{
		switch (RuleViolation->ViolatedRule.Get()->GetDefaultObject<ULintRule>()->RuleSeverity)
		{
			case ELintRuleSeverity::Error:
				NumErrors++;
				break;
			case ELintRuleSeverity::Warning:
				NumWarnings++;
				break;
			case ELintRuleSeverity::Info:
				break;
			//case ELintRuleSeverity::Ignore:
			default:
				break;			
		}
	}

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.Padding(PaddingAmount)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(PaddingAmount)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(PaddingAmount)
				[
					SNew(SExpandableArea)
					.InitiallyCollapsed(false)
					.HeaderContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(AssetName)
							.TextStyle(FLinterStyle::Get(), "Linter.Report.AssetName")
						]
					]
					.BodyContent()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Top)
						.Padding(PaddingAmount)
						[
							SNew(SBox)
							.WidthOverride(96.0f)
							.HeightOverride(96.0f)
							[
								AssetThumbnail->MakeThumbnailWidget()
							]
						]
						+ SHorizontalBox::Slot()
						.Padding(PaddingAmount)
						[
							SNew(SVerticalBox)
							+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Left)
							.Padding(PaddingAmount)
							[
								SNew(SHyperlink)
								.Text(AssetPath)
								.OnNavigate_Lambda([&]()
								{
									FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
									FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
									TArray<FAssetData> AssetDatas;
									AssetDatas.Push(AssetData.Get());
									ContentBrowserModule.Get().SyncBrowserToAssets(AssetDatas);
								})
							]
						+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(PaddingAmount)
							[
								SNew(STextBlock)
								.Visibility((RuleViolations.Get().Num() > 0) ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed)
								.Text(FText::FormatNamed(LOCTEXT("ErrorWarningDisplay", "{NumErrors} {NumErrors}|plural(one=Error,other=Errors), {NumWarnings} {NumWarnings}|plural(one=Warning,other=Warnings)"), TEXT("NumErrors"), NumErrors, TEXT("NumWarnings"), NumWarnings))
							]
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(PaddingAmount)
							[
								SNew(SLintReportAssetErrorList)
								.RuleViolations(RuleViolations)
							]
						]
					]
				]
			]
		]
	];
}

#undef LOCTEXT_NAMESPACE
