// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once
#include "Widgets/SCompoundWidget.h"
#include "LintRule.h"

class SLintReportRuleError : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SLintReportRuleError)
	{
	}
	SLATE_ATTRIBUTE(TSharedPtr<FLintRuleViolation>, RuleViolation)
	
	SLATE_END_ARGS()

	TAttribute<TSharedPtr<FLintRuleViolation>> RuleViolation;


public:
	
	void Construct(const FArguments& Args);
};