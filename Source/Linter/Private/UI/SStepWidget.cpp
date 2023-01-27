// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#include "UI/SStepWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Text/SRichTextBlock.h"

bool SStepWidget::IsStepCompleted(bool bAllowWarning /*= true*/)
{
	EStepStatus Status = StepStatus.Get();
	if (bAllowWarning && Status == EStepStatus::Warning)
	{
		return true;
	}

	return Status == EStepStatus::Success;
}

void SStepWidget::Construct(const FArguments& Args)
{
	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

	StepStatus = Args._StepStatus;
	OnPerformAction = Args._OnPerformAction;
	StepActionText = Args._StepActionText;
	ShowStepStatusIcon = Args._ShowStepStatusIcon;

	// Visibility lambda based on whether step is in progress
	auto VisibleIfInProgress = [this]()
	{
		return StepStatus.Get(EStepStatus::NoStatus) == EStepStatus::InProgress ? EVisibility::Visible : EVisibility::Collapsed;
	};

	// Enabled lambda based on whether this widget has a step status that requires action
	auto EnabledBasedOnStepStatus = [this]() -> bool
	{
		switch (StepStatus.Get(EStepStatus::NoStatus))
		{
			case EStepStatus::NoStatus:
			case EStepStatus::InProgress:
			case EStepStatus::Success:
				return false;
			case EStepStatus::Unknown:
			case EStepStatus::Warning:
			case EStepStatus::Error:
			case EStepStatus::NeedsUpdate:
				return true;
		}
		return false;
	};

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
				SNew(SHorizontalBox)
				// Status Image
				+ SHorizontalBox::Slot()
				.Padding(PaddingAmount)
				.AutoWidth()
				[
					SNew(SImage)
					.Visibility_Lambda([&]() { return StepStatus.Get(EStepStatus::NoStatus) == EStepStatus::NoStatus || !ShowStepStatusIcon.Get(true) ? EVisibility::Collapsed : EVisibility::Visible; })
					.Image_Lambda([&]()
					{
						switch (StepStatus.Get(EStepStatus::NoStatus))
						{
							case NoStatus:
							case Unknown:
								return FLinterStyle::Get()->GetBrush("Linter.Step.Unknown");
							case InProgress:
							case NeedsUpdate:
								return FLinterStyle::Get()->GetBrush("Linter.Step.Working");
							case Warning:
								return FLinterStyle::Get()->GetBrush("Linter.Step.Warning");
							case Error:
								return FLinterStyle::Get()->GetBrush("Linter.Step.Error");
							case Success:
								return FLinterStyle::Get()->GetBrush("Linter.Step.Good");
						}
	
						return FLinterStyle::Get()->GetBrush("Linter.Step.Unknown");
					})
				]
				// Template thumbnail image
				+ SHorizontalBox::Slot()
				.Padding(4.0)
				.AutoWidth()
				.VAlign(VAlign_Top)
				[
					SNew(SImage)
					.Visibility(Args._Icon.IsSet() ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed)
					.Image(Args._Icon)
				]
				// Template name and description
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(PaddingAmount)
					[
						SNew(STextBlock)
						.Text(Args._StepName)
						.TextStyle(FLinterStyle::Get(), "Linter.Report.RuleTitle")
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(PaddingAmount)
					[
						SNew(SRichTextBlock)
						.Text(Args._StepDesc)
						.TextStyle(FLinterStyle::Get(), "Linter.Report.DescriptionText")
						.AutoWrapText(true)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(PaddingAmount)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.IsEnabled_Lambda(EnabledBasedOnStepStatus)
							.Visibility_Lambda([&]() { return StepStatus.Get(EStepStatus::NoStatus) == EStepStatus::NoStatus ? EVisibility::Collapsed : EVisibility::Visible; })
							.OnClicked_Lambda([&]()
							{
								FScopedSlowTask SlowTask(1.0f, StepActionText.Get(FText()));
								SlowTask.MakeDialog();

								OnPerformAction.ExecuteIfBound(SlowTask);
								return FReply::Handled();
							})
							[
								SNew(STextBlock)
								.Text(StepActionText)
							]
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SThrobber)
							.Visibility_Lambda(VisibleIfInProgress)
						]
					]
				]
			]
		]
	];
}
