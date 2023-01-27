// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class SLintReportRuleErrorList : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLintReportRuleErrorList)
	{
	}
	SLATE_ATTRIBUTE(TArray<TSharedPtr<FLintRuleViolation>>, RuleViolations)

	SLATE_END_ARGS()

	TAttribute<TArray<TSharedPtr<FLintRuleViolation>>> RuleViolations;

public:
	
	void Construct(const FArguments& Args);

};