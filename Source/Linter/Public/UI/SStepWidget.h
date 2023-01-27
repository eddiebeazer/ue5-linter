// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "EditorStyleSet.h"
#include "LinterStyle.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Types/SlateStructs.h"
#include "Misc/ScopedSlowTask.h"


enum EStepStatus {
	NoStatus,
	Unknown,
	InProgress,
	NeedsUpdate,
	Warning,
	Error,
	Success
};

// Called when a step's action is invoked
DECLARE_DELEGATE_OneParam(FOnStepPerformAction, FScopedSlowTask&);

/* Widget that represents a 'step' or 'choice' in the PaCK wizard. */
class SStepWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SStepWidget)
		: _StepStatus(EStepStatus::NoStatus)
		, _ShowStepStatusIcon(true)
	{
	}

		/** Name to display for this step. */
		SLATE_ATTRIBUTE(FText, StepName)

		/** Description to display for this step. */
		SLATE_ATTRIBUTE(FText, StepDesc)

		/** Text to display within the action button for this step. */
		SLATE_ATTRIBUTE(FText, StepActionText)

		/** Slate Brush to use as the thumbnail icon for this step. */
		SLATE_ATTRIBUTE(const FSlateBrush*, Icon)

		/** Current status for this step. */
		SLATE_ATTRIBUTE(EStepStatus, StepStatus)

		/** Current status for this step. */
		SLATE_ATTRIBUTE(bool, ShowStepStatusIcon)

		/** Delegate to fire when this step's action is invoked. */
		SLATE_EVENT(FOnStepPerformAction, OnPerformAction)

	SLATE_END_ARGS()

public:
	TAttribute<EStepStatus> StepStatus;
	TAttribute<FText> StepActionText;
	FOnStepPerformAction OnPerformAction;
	TAttribute<bool> ShowStepStatusIcon;

	bool IsStepCompleted(bool bAllowWarning = true);
	
	void Construct(const FArguments& Args);
};