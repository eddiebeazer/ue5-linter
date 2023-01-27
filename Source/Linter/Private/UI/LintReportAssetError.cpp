// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "UI/LintReportAssetError.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Input/SHyperlink.h"
#include "LintRule.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "LintReport"


void SLintReportAssetError::Construct(const FArguments& Args)
{
	RuleViolation = Args._RuleViolation;
	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

	ULintRule* LintRule = RuleViolation.Get()->ViolatedRule.Get()->GetDefaultObject<ULintRule>();
	check(LintRule != nullptr);

	const FSlateBrush* RuleIcon = nullptr;
	bool bHasURL = !LintRule->RuleURL.IsEmpty();

	switch (LintRule->RuleSeverity)
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
			+ SHorizontalBox::Slot()
			.Padding(PaddingAmount)
			.AutoWidth()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LintRule->RuleTitle)
				.TextStyle(FLinterStyle::Get(), "Linter.Report.RuleTitle")
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(16.0f)
				.HeightOverride(16.0f)
				[
					SNew(SImage)
					.Cursor(EMouseCursor::Hand)
					.Visibility(bHasURL ? EVisibility::Visible : EVisibility::Collapsed)
					.OnMouseButtonDown_Lambda([&](const FGeometry& Geo, const FPointerEvent& Event) { FPlatformProcess::LaunchURL(*RuleViolation.Get()->ViolatedRule.Get()->GetDefaultObject<ULintRule>()->RuleURL, NULL, NULL); return FReply::Handled(); })
					.Image(FLinterStyle::Get()->GetBrush("Linter.Report.Link"))
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.Padding(20.0f, PaddingAmount, PaddingAmount, PaddingAmount)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(LintRule->RuleDescription)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.Padding(20.0f, PaddingAmount, PaddingAmount, PaddingAmount)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(RuleViolation.Get()->RecommendedAction)
		]
	];
}

