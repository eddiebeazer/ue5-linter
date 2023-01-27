// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "UI/LintReport.h"
#include "UI/LintReportAssetError.h"

#include "LintRule.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/SBoxPanel.h"
#include "LintRuleSet.h"
#include "UI/LintReportAssetErrorList.h"
#include "UI/LintReportAssetDetails.h"
#include "AssetThumbnail.h"
#include "Containers/Map.h"
#include "LinterSettings.h"
#include "Misc/ScopedSlowTask.h"
#include "Widgets/Layout/SSpacer.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonValue.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Misc/FileHelper.h"
#include "Widgets/Input/SComboButton.h"
#include "UI/LintReportRuleDetails.h"

#define LOCTEXT_NAMESPACE "Linter"

void SLintReport::Construct(const FArguments& Args)
{
	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			.Padding(PaddingAmount)
			[
				SNew(SButton)
				.Text(LOCTEXT("Rescan", "Rescan"))
				.OnClicked_Lambda([this]() -> FReply { Rebuild(LastUsedRuleSet); return FReply::Handled(); })
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.AutoWidth()
			.Padding(PaddingAmount)
			[
				SAssignNew(ResultsTextBlockPtr, STextBlock)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Center)
			.FillWidth(1.0f)
			.Padding(PaddingAmount)
			[
				SNew(SSpacer)
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(PaddingAmount)
			[
				SNew(SButton)
				.Text(LOCTEXT("ExportToJSON", "Export To JSON"))
				.OnClicked_Lambda([this]() -> FReply
				{ 
					IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

					const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

					const FText Title = LOCTEXT("ExportToJsonTitle", "Export Lint Report as JSON");
					const FString FileTypes = TEXT("Json (*.json)|*.json");

					FDateTime Now = FDateTime::Now();
					FString Output = TEXT("lint-report-") + FDateTime::Now().ToString() + TEXT(".json");
					
					FString DefaultPath = FPaths::ProjectSavedDir() / TEXT("LintReports");
					DefaultPath = FPaths::ConvertRelativePathToFull(DefaultPath);
					IFileManager::Get().MakeDirectory(*DefaultPath, true);

					TArray<FString> OutFilenames;
					DesktopPlatform->SaveFileDialog(
						ParentWindowWindowHandle,
						Title.ToString(),
						DefaultPath,
						Output,
						FileTypes,
						EFileDialogFlags::None,
						OutFilenames
					);

					if (OutFilenames.Num() > 0)
					{
						FString WritePath = FPaths::ConvertRelativePathToFull(OutFilenames[0]);
						FFileHelper::SaveStringToFile(JsonReport, *WritePath);
						FPlatformProcess::LaunchURL(*WritePath, TEXT(""), nullptr);
					}

					return FReply::Handled();
				})
			]
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			.AutoWidth()
			.Padding(PaddingAmount)
			[
				SNew(SButton)
				.Text(LOCTEXT("ExportToHTML", "Export To HTML"))
				.OnClicked_Lambda([this]() -> FReply
				{ 
					IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

					const void* ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

					const FText Title = LOCTEXT("ExportToHTMLTitle", "Export Lint Report as HTML");
					const FString FileTypes = TEXT("HTML (*.html)|*.html");

					FDateTime Now = FDateTime::Now();
					FString Output = TEXT("lint-report-") + FDateTime::Now().ToString() + TEXT(".html");
					
					FString DefaultPath = FPaths::ProjectSavedDir() / TEXT("LintReports");
					DefaultPath = FPaths::ConvertRelativePathToFull(DefaultPath);
					IFileManager::Get().MakeDirectory(*DefaultPath, true);

					TArray<FString> OutFilenames;
					DesktopPlatform->SaveFileDialog(
						ParentWindowWindowHandle,
						Title.ToString(),
						DefaultPath,
						Output,
						FileTypes,
						EFileDialogFlags::None,
						OutFilenames
					);

					if (OutFilenames.Num() > 0)
					{
						FString WritePath = FPaths::ConvertRelativePathToFull(OutFilenames[0]);
						FFileHelper::SaveStringToFile(HTMLReport, *WritePath);
						FPlatformProcess::LaunchURL(*WritePath, TEXT(""), nullptr);
					}

					return FReply::Handled();
				})
			]
		]
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		.FillHeight(1.0f)
		.Padding(PaddingAmount)
		[
			SAssignNew(AssetDetailsScrollBoxPtr, SScrollBox)
			.ScrollBarAlwaysVisible(true)
		]
		+ SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		.FillHeight(1.0f)
		.Padding(PaddingAmount)
		[
			SAssignNew(RuleDetailsScrollBoxPtr, SScrollBox)
			.ScrollBarAlwaysVisible(true)
			.Visibility(EVisibility::Collapsed)
		]
		// Bottom panel
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("NoBorder"))
			.Padding(FMargin(4.0f, 0.0f, 4.0f, 2.0f))
			//.Visibility_Lambda([&]() { return AssetErrorLists.Num() > 0 ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed; })
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(2.0f, 0.0f, 2.0f, 2.0f))
				[

					SNew(SHorizontalBox)
					// View mode combo button
					+SHorizontalBox::Slot()
					.FillWidth(1.f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Right)
					[
						SAssignNew( ViewOptionsComboButton, SComboButton )
						.ContentPadding(0)
						.ForegroundColor_Lambda([&]() { return ViewOptionsComboButton->IsHovered() ? FEditorStyle::GetSlateColor("InvertedForeground") : FEditorStyle::GetSlateColor("DefaultForeground"); })
						.ButtonStyle( FEditorStyle::Get(), "ToggleButton" ) // Use the tool bar item style for this button
						.OnGetMenuContent( this, &SLintReport::GetViewButtonContent )
						.ButtonContent()
						[
							SNew(SHorizontalBox)
							+SHorizontalBox::Slot()
							.AutoWidth()
							.VAlign(VAlign_Center)
							[
								SNew(SImage).Image( FEditorStyle::GetBrush("GenericViewButton") )
							]
 
							+SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(2, 0, 0, 0)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock).Text( LOCTEXT("ViewButton", "View Options") )
							]
						]
					]
				]
			]
		]
	];
}

void SLintReport::Rebuild(const ULintRuleSet* SelectedLintRuleSet)
{
	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");

	NumErrors = 0;
	NumWarnings = 0;
	bHasRanReport = false;

	if (SelectedLintRuleSet == nullptr)
	{
		SelectedLintRuleSet = GetDefault<ULinterSettings>()->DefaultLintRuleSet.LoadSynchronous();
	}

	check(SelectedLintRuleSet != nullptr);
	LastUsedRuleSet = SelectedLintRuleSet;

	AssetDetailsScrollBoxPtr->ClearChildren();
	RuleDetailsScrollBoxPtr->ClearChildren();
	RuleViolations.Reset();

	FScopedSlowTask SlowTask(0, LOCTEXT("LintingInProgress", "Linting Assets..."));
	SlowTask.MakeDialog(false);

	FLinterModule& LinterModule = FModuleManager::LoadModuleChecked<FLinterModule>(TEXT("Linter"));
	TArray<FString> LintPaths = LinterModule.GetDesiredLintPaths();

	RuleViolations = SelectedLintRuleSet->LintPathShared(LintPaths, &SlowTask);

	for (TSharedPtr<FLintRuleViolation> Violation : RuleViolations)
	{
		if (Violation->ViolatedRule->GetDefaultObject<ULintRule>()->RuleSeverity <= ELintRuleSeverity::Error)
		{
			NumErrors++;
		}
		else
		{
			NumWarnings++;
		}
	}

	TArray<UObject*> UniqueViolators = FLintRuleViolation::AllRuleViolationViolators(RuleViolations);
	TSharedPtr<FAssetThumbnailPool> ThumbnailPool = MakeShareable(new FAssetThumbnailPool(UniqueViolators.Num()));

	TSharedPtr<FJsonObject> RootJsonObject = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> ViolatorJsonObjects;

	for (UObject* Violator : UniqueViolators)
	{
		// We might as well build JSON data here as we're iterating through all our rule violations anyway
		TSharedPtr<FJsonObject> AssetJsonObject = MakeShareable(new FJsonObject);
		TArray<TSharedPtr<FLintRuleViolation>> UniqueViolatorViolations = FLintRuleViolation::AllRuleViolationsWithViolatorShared(RuleViolations, Violator);

		FAssetData AssetData;
		if (UniqueViolatorViolations.Num() > 0)
		{
			AssetData = UniqueViolatorViolations[0]->ViolatorAssetData;
			AssetJsonObject->SetStringField(TEXT("ViolatorAssetName"), AssetData.AssetName.ToString());
			AssetJsonObject->SetStringField(TEXT("ViolatorAssetPath"), AssetData.ObjectPath.ToString());
			AssetJsonObject->SetStringField(TEXT("ViolatorFullName"), AssetData.GetFullName());
			//@TODO: Thumbnail export?

			TArray<TSharedPtr<FJsonValue>> RuleViolationJsonObjects;

			for (TSharedPtr<FLintRuleViolation> Violation : UniqueViolatorViolations)
			{
				ULintRule* LintRule = Violation->ViolatedRule->GetDefaultObject<ULintRule>();
				check(LintRule != nullptr);

				TSharedPtr<FJsonObject> RuleJsonObject = MakeShareable(new FJsonObject);
				RuleJsonObject->SetStringField(TEXT("RuleGroup"), LintRule->RuleGroup.ToString());
				RuleJsonObject->SetStringField(TEXT("RuleTitle"), LintRule->RuleTitle.ToString());
				RuleJsonObject->SetStringField(TEXT("RuleDesc"), LintRule->RuleDescription.ToString());
				RuleJsonObject->SetStringField(TEXT("RuleURL"), LintRule->RuleURL);
				RuleJsonObject->SetNumberField(TEXT("RuleSeverity"), (int32)LintRule->RuleSeverity);
				RuleJsonObject->SetStringField(TEXT("RuleRecommendedAction"), Violation->RecommendedAction.ToString());
				RuleViolationJsonObjects.Push(MakeShareable(new FJsonValueObject(RuleJsonObject)));
			}

			AssetJsonObject->SetArrayField(TEXT("Violations"), RuleViolationJsonObjects);
		}

		ViolatorJsonObjects.Add(MakeShareable(new FJsonValueObject(AssetJsonObject)));

		AssetDetailsScrollBoxPtr.Get()->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(PaddingAmount)
		[
			SNew(SLintReportAssetDetails)
			.AssetData(AssetData)
			.RuleViolations(UniqueViolatorViolations)
			.ThumbnailPool(ThumbnailPool)
		];
	}

	TMultiMap<const ULintRule*, TSharedPtr<FLintRuleViolation>> ViolationsMappedByRule = FLintRuleViolation::AllRuleViolationsMappedByViolatedLintRuleShared(RuleViolations);
	TSharedPtr<FAssetThumbnailPool> RuleThumbnailPool = MakeShareable(new FAssetThumbnailPool(ViolationsMappedByRule.Num())); // Incase we ever want to render 'rule thumbnails' in the future

	TArray<const ULintRule*> UniqueRules;
	ViolationsMappedByRule.GetKeys(UniqueRules);

	for (const ULintRule* BrokenRule : UniqueRules)
	{
		TArray<TSharedPtr<FLintRuleViolation>> ViolatorsOfBrokenRule;
		ViolationsMappedByRule.MultiFind(BrokenRule, ViolatorsOfBrokenRule);

		if (ViolatorsOfBrokenRule.Num() > 0)
		{
			RuleDetailsScrollBoxPtr.Get()->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.Padding(PaddingAmount)
			[
				SNew(SLintReportRuleDetails)
				.RuleViolations(ViolatorsOfBrokenRule)
				.ThumbnailPool(RuleThumbnailPool)
			];
		}
	}

	// Save off our JSON to a string
	RootJsonObject->SetArrayField(TEXT("Violators"), ViolatorJsonObjects);
	JsonReport.Empty();
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonReport);
	FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), Writer);

	// Update Summary Text Block
	int32 NumAssets = UniqueViolators.Num();
	FText ResultsSummary = FText::FormatNamed(LOCTEXT("ErrorWarningDisplay", "{NumAssets} {NumAssets}|plural(one=Asset,other=Assets), {NumErrors} {NumErrors}|plural(one=Error,other=Errors), {NumWarnings} {NumWarnings}|plural(one=Warning,other=Warnings)"), TEXT("NumAssets"), NumAssets, TEXT("NumErrors"), NumErrors, TEXT("NumWarnings"), NumWarnings);
	ResultsTextBlockPtr->SetText(ResultsSummary);

	// Prepare the HTML Export
	FString TemplatePath = FPaths::Combine(*IPluginManager::Get().FindPlugin(TEXT("Linter"))->GetBaseDir(), TEXT("Resources"), TEXT("LintReportTemplate.html"));

	if (FFileHelper::LoadFileToString(HTMLReport, *TemplatePath))
	{
		HTMLReport.ReplaceInline(TEXT("{% TITLE %}"), *FPaths::GetBaseFilename(FPaths::GetProjectFilePath()));
		HTMLReport.ReplaceInline(TEXT("{% RESULTS %}"), *ResultsSummary.ToString());
		HTMLReport.ReplaceInline(TEXT("{% LINT_REPORT %}"), *JsonReport);
	}

	bHasRanReport = true;
}

TSharedRef<SWidget> SLintReport::GetViewButtonContent()
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/true, nullptr, TSharedPtr<FExtender>(), /*bCloseSelfOnly=*/ true);

	MenuBuilder.BeginSection("AssetViewType", LOCTEXT("ViewTypeHeading", "View Type"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("RuleFirstOption", "Assets by Guideline"),
			LOCTEXT("RuleFirstOptionTooltip", "View failing assets as a categorized list of guidelines."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([&]()
				{ 
					if (AssetDetailsScrollBoxPtr.IsValid())
					{
						AssetDetailsScrollBoxPtr->SetVisibility(EVisibility::SelfHitTestInvisible);
					}
					if (RuleDetailsScrollBoxPtr.IsValid())
					{
						RuleDetailsScrollBoxPtr->SetVisibility(EVisibility::Collapsed);
					}
				}),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([&]() { return AssetDetailsScrollBoxPtr.IsValid() && AssetDetailsScrollBoxPtr->GetVisibility().IsVisible(); })
			),
			NAME_None,
			EUserInterfaceActionType::RadioButton
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("AssetsFirstOption", "Guidelines by Asset"),
			LOCTEXT("AssetsFirstOptionTooltip", "View the failing assets one at a time with all their respective errors."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateLambda([&]()
				{ 
					if (AssetDetailsScrollBoxPtr.IsValid())
					{
						AssetDetailsScrollBoxPtr->SetVisibility(EVisibility::Collapsed);
					}
					if (RuleDetailsScrollBoxPtr.IsValid())
					{
						RuleDetailsScrollBoxPtr->SetVisibility(EVisibility::SelfHitTestInvisible);
					}
				}),
				FCanExecuteAction(),
				FIsActionChecked::CreateLambda([&]() { return RuleDetailsScrollBoxPtr.IsValid() && RuleDetailsScrollBoxPtr->GetVisibility().IsVisible(); })
			),
			NAME_None,
			EUserInterfaceActionType::RadioButton
		);
	}

	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE