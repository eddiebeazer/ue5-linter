// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#include "UI/LintReportruleErrorList.h"
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

#define LOCTEXT_NAMESPACE "LintReport"

void SLintReportRuleErrorList::Construct(const FArguments& Args)
{
	RuleViolations = Args._RuleViolations;
	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

	ChildSlot
	[
		SNew(SListView<TSharedPtr<FLintRuleViolation>>)
		.SelectionMode(ESelectionMode::None)
		.ListItemsSource(&RuleViolations.Get())
		.OnGenerateRow_Lambda([&](TSharedPtr<FLintRuleViolation> InItem, const TSharedRef<STableViewBase>& OwnerTable)
		{
		return SNew(STableRow<TSharedPtr<FLintRuleViolation>>, OwnerTable)
			[
				SNew(SLintReportRuleError)
				.RuleViolation(InItem)
			];
		})
	];
}
