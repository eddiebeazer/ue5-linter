// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "UI/LintReportRuleDetails.h"
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
#include "UI/LintReportRuleErrorList.h"
#include "LintRule.h"
#include "AssetThumbnail.h"



#define LOCTEXT_NAMESPACE "LintReport"

void SLintReportRuleDetails::Construct(const FArguments& Args)
{
	RuleViolations = Args._RuleViolations;
	ThumbnailPool = Args._ThumbnailPool;

	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

	check(RuleViolations.Get().Num() > 0 );
	ULintRule* BrokenRule = (RuleViolations.Get())[0]->ViolatedRule.GetDefaultObject();
	check(BrokenRule != nullptr);

	FText RuleName = BrokenRule->RuleTitle;
	FText RuleDesc = BrokenRule->RuleDescription;

	RuleURL = BrokenRule->RuleURL;

	const FSlateBrush* RuleIcon = nullptr;
	switch (BrokenRule->RuleSeverity)
	{
	case ELintRuleSeverity::Error:
		RuleIcon = FLinterStyle::Get()->GetBrush("Linter.Report.Error");
		break;
	case ELintRuleSeverity::Warning:
		RuleIcon = FLinterStyle::Get()->GetBrush("Linter.Report.Warning");
		break;
	case ELintRuleSeverity::Info:
		RuleIcon = FLinterStyle::Get()->GetBrush("Linter.Report.Info");
		break;
		//case ELintRuleSeverity::Ignore:
	default:
		break;
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	RuleAssetData = AssetRegistry.GetAssetByObjectPath(FName(*BrokenRule->GetPathName()), true);
	FText RuleAssetPath;
	if (RuleAssetData.IsValid())
	{
		RuleAssetPath = FText::FromName(RuleAssetData.PackagePath);
	}



	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush("NoBorder"))
		.Padding(PaddingAmount)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
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
						// Rule Thumbnail
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Top)
						.Padding(PaddingAmount)
						[
							SAssignNew(ThumbnailBox, SBox)
							.WidthOverride(96.0f)
							.HeightOverride(96.0f)
							.Visibility(RuleAssetData.IsValid() ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
						]
						// Rule Icon
						+ SHorizontalBox::Slot()
						.Padding(PaddingAmount)
						.AutoWidth()
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Left)
						[
							SNew(SBox)
							.WidthOverride(14.0f)
							.HeightOverride(14.0f)
							[
								SNew(SImage)
								.Image(RuleIcon)
							]
						]
						// Rule Name
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(RuleName)
							.TextStyle(FLinterStyle::Get(), "Linter.Report.AssetName")
						]
						// Link to Rule URL
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(8.0f, 0.0)
						[
							SNew(SImage)
							.Image(FLinterStyle::Get()->GetBrush("Linter.Report.Link"))
							.Cursor(EMouseCursor::Hand)
							.Visibility(RuleURL.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
							.OnMouseButtonDown_Lambda([&](const FGeometry& Geo, const FPointerEvent& Event) { FPlatformProcess::LaunchURL(*RuleURL, NULL, NULL); return FReply::Handled(); })
						]
						// Link to Rule Definition Asset
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(8.0f, 0.0)
						[
							SNew(SImage)
							.Image(FLinterStyle::Get()->GetBrush("Linter.Report.Link"))
							.Cursor(EMouseCursor::Hand)
							.Visibility(EVisibility::Collapsed)
							.OnMouseButtonDown_Lambda([&](const FGeometry& Geo, const FPointerEvent& Event)
							{
								FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
								ContentBrowserModule.Get().SyncBrowserToAssets(TArray<FAssetData>({ RuleAssetData }));
								return FReply::Handled();
							})
						]
						// Asset Count
						+ SHorizontalBox::Slot()
						.VAlign(VAlign_Center)
						.AutoWidth()
						.Padding(8.0f, 0.0)
						[
							SNew(SHyperlink)
							.Text(FText::FormatNamed(LOCTEXT("AssetCountDisplay", "{NumAssets} {NumAssets}|plural(one=Asset,other=Assets)"), TEXT("NumAssets"), RuleViolations.Get().Num()))
							.OnNavigate_Lambda([&]()
							{
								FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
								TArray<FAssetData> AssetDatas;

								for (const TSharedPtr<FLintRuleViolation>& RuleViolation : RuleViolations.Get())
								{
									AssetDatas.Push(RuleViolation->ViolatorAssetData);
								}
								
								ContentBrowserModule.Get().SyncBrowserToAssets(AssetDatas);
							})
						]
					]
					.BodyContent()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.Padding(PaddingAmount)
						.HAlign(HAlign_Left)
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text(RuleDesc)
							.AutoWrapText(true)
							.TextStyle(FLinterStyle::Get(), "Linter.Report.AssetName")
						]
						+ SVerticalBox::Slot()
						.Padding(PaddingAmount)
						.HAlign(HAlign_Left)
						.AutoHeight()
						[
							SNew(SLintReportRuleErrorList)
							.RuleViolations(RuleViolations)
						]
					]
				]
			]
		]
	];

	if (RuleAssetData.IsValid())
	{
		const TSharedPtr<FAssetThumbnail> RuleThumbnail = MakeShareable(new FAssetThumbnail(RuleAssetData, 96, 96, ThumbnailPool.Get()));
		ThumbnailBox->SetContent(RuleThumbnail->MakeThumbnailWidget());
	}
}
