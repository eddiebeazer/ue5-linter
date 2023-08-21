// Copyright 2015-2017 by Gamemakin LLC
#include "UI/LintWizard.h"

#include "CoreGlobals.h"
#include "Delegates/Delegate.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"
#include "AssetData.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Layout/SScrollBox.h"
#include "IUATHelperModule.h"
#include "Misc/FeedbackContext.h"
#include "Framework/Notifications/NotificationManager.h"
#include "AssetThumbnail.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "DesktopPlatformModule.h"
#include "FileHelpers.h"
#include "IDesktopPlatform.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"

#include "LinterStyle.h"
#include "LintRuleSet.h"
#include "LinterSettings.h"
#include "UI/SAssetLinkWidget.h"

// This is only valid within a single file, and must be undefined before the end of the file
#define LOCTEXT_NAMESPACE "Linter"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLintWizard::Construct(const FArguments& InArgs)
{
	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");


	RuleSets = TArray<TSharedPtr<FAssetData>>();

	// Try to load the default rule set
	ULintRuleSet* DefaultRuleSet = GetDefault<ULinterSettings>()->DefaultLintRuleSet.LoadSynchronous();

	// Even though this happens on module startup, we try force loading all rule sets again in case any new unloaded rule sets have been added
	// or for some reason our existing rule sets were unloaded from memory
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> FoundRuleSets;
	AssetRegistry.GetAssetsByClass(ULintRuleSet::StaticClass()->GetFName(), FoundRuleSets, true);

	// Attempt to get all RuleSets in memory so that linting tools are better aware of them
	for (const FAssetData& RuleSetData : FoundRuleSets)
	{
		ULintRuleSet* LoadedRuleSet = Cast<ULintRuleSet>(RuleSetData.GetAsset());
		if (LoadedRuleSet != nullptr)
		{
			FAssetData* newData = new FAssetData;
			*newData = RuleSetData;
			RuleSets.Push(MakeShareable(newData));
			if (LoadedRuleSet == DefaultRuleSet)
			{
				SelectedRuleSet = RuleSets.Last();
			}
		}
	}

	if (!SelectedRuleSet.IsValid() && RuleSets.Num() > 0)
	{
		SelectedRuleSet = RuleSets[0];
	}

	ChildSlot
	[
		SNew(SBorder)
		.Padding(18)
		.BorderImage(FAppStyle::GetBrush("Docking.Tab.ContentAreaBrush"))
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			[
				SAssignNew(MainWizard, SWizard)
				.ShowPageList(false)
				.ShowCancelButton(false)
				.ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				.CancelButtonStyle(FAppStyle::Get(), "FlatButton.Default")
				.FinishButtonStyle(FAppStyle::Get(), "FlatButton.Success")
				.ButtonTextStyle(FAppStyle::Get(), "LargeText")
				.CanFinish(true)
				.FinishButtonText(LOCTEXT("FinishButtonText", "Close"))
				.OnFinished_Lambda([&]()
				{
					FGlobalTabmanager::Get()->FTabManager::TryInvokeTab(FName("LinterTab"))->RequestCloseTab();
				})
				+ SWizard::Page()
				.CanShow_Lambda([&]() { return RuleSets.Num() > 0; })
				[
					SNew(SVerticalBox)
					// Title
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0)
					[
						SNew(STextBlock)
						.TextStyle( FAppStyle::Get(), "NewClassDialog.PageTitle" )
						.Text(LOCTEXT("LinterSelectionTitle", "Linter Rule Set Selection"))
					]
					// Title spacer
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 2, 0, 8)
					[
						SNew(SSeparator)
					]
					// Linter Selection
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(PaddingAmount)
					[
						SAssignNew(RuleSetSelectionComboBox, SComboBox<TSharedPtr<FAssetData>>)
						.OptionsSource(&RuleSets)
						.InitiallySelectedItem(SelectedRuleSet)
						.OnGenerateWidget_Lambda([&](TSharedPtr<FAssetData> LintRuleSet)
						{ 
							ULintRuleSet* RuleSet = Cast<ULintRuleSet>(LintRuleSet->GetAsset());
							if (RuleSet != nullptr)
							{
								return SNew(STextBlock).Text(RuleSet->RuleSetDescription.IsEmpty() ? FText::FromString(RuleSet->GetPathName()) : RuleSet->RuleSetDescription);
							}
							return SNew(STextBlock).Text(FText::FromString(TEXT("This Lint Rule Set Failed To Load? Uhhhh....")));							
						})
						.OnSelectionChanged_Lambda([&](TSharedPtr<FAssetData> Item, ESelectInfo::Type SelectInfo) { SelectedRuleSet = Item; RuleSetSelectionComboBox->RefreshOptions(); })
						.ContentPadding(4.0f)
						[
							SNew(STextBlock)
							.Text_Lambda([&]() { return SelectedRuleSet->GetAsset() != nullptr ? Cast<ULintRuleSet>(SelectedRuleSet->GetAsset())->RuleSetDescription : FText::GetEmpty(); })
						]
					]
				]
				// Lint Report
				+ SWizard::Page()
				.OnEnter(this, &SLintWizard::OnLintReportEntered)
				.CanShow_Lambda([&]() { return RuleSets.Num() >= 1 && SelectedRuleSet.IsValid(); })
				[
					SNew(SVerticalBox)
					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0)
					[
						SNew(STextBlock)
						.TextStyle( FAppStyle::Get(), "NewClassDialog.PageTitle" )
						.Text(LOCTEXT("LinterReportTitle", "Lint Report"))
					]
					// Marketplace No Errors Required Text
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(PaddingAmount)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("MarketplaceNoErrorsRequired", "The Epic Marketplace requires you to have zero linting errors before submission and approval."))
						.Visibility_Lambda([&]() { return (LintReport.IsValid() && LintReport->bHasRanReport && LintReport->NumErrors > 0 && LintReport->LastUsedRuleSet != nullptr && LintReport->LastUsedRuleSet->bShowMarketplacePublishingInfoInLintWizard) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
					]
					// Title spacer
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 2, 0, 8)
					[
						SNew(SSeparator)
					]
					// Linter Report
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					.Padding(PaddingAmount)
					[
						SAssignNew(LintReport, SLintReport)
					]
				]
				// Marketplace Info Page
				+ SWizard::Page()
				.OnEnter(this, &SLintWizard::OnMarketplaceRecommendationsEntered)
				.CanShow_Lambda([&]() { return LintReport.IsValid() && LintReport->bHasRanReport && LintReport->NumErrors <= 0 && LintReport->LastUsedRuleSet != nullptr && LintReport->LastUsedRuleSet->bShowMarketplacePublishingInfoInLintWizard; })
				[
					SNew(SVerticalBox)
					// Title
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0)
					[
						SNew(STextBlock)
						.TextStyle( FAppStyle::Get(), "NewClassDialog.PageTitle" )
						.Text(LOCTEXT("MarketplaceInfoTitle", "Marketplace Recommendations"))
					]
					// Title spacer
					+SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 2, 0, 8)
					[
						SNew(SSeparator)
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							SNew(SVerticalBox)
							// Disclaimer
							+ SVerticalBox::Slot()
							.VAlign(VAlign_Top)
							.AutoHeight()
							[
								SNew(SBorder)
								.BorderImage(FAppStyle::GetBrush("NoBorder"))
								.Padding(PaddingAmount)
								[
									SNew(SBorder)
									.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
									.Padding(PaddingAmount)
									[
										SNew(SHorizontalBox)
										// Info image
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(28.0f, 8.0f)
										[
											SNew(SImage).Image(FLinterStyle::Get()->GetBrush("Linter.Report.Info"))
										]
										// Disclaimer text
										+SHorizontalBox::Slot()
										.VAlign(VAlign_Center)
										.FillWidth(1.0f)
										[
											SNew(SRichTextBlock)
											.Text(LOCTEXT("SuccessDisclaimer", "This product has successfully passed the Marketplace Guidlines Linter scan. Please note, however, that this does not guarantee this product's acceptance onto the Marketplace. We also recommend that you take the following actions listed below."))
											.AutoWrapText(true)
											.TextStyle(FLinterStyle::Get(), "Linter.Report.DescriptionText")
										]
									]
								]
							]
							// Fix Up Redirectors step widget
							+ SVerticalBox::Slot()
							.AutoHeight()
							.VAlign(VAlign_Fill)
							.HAlign(HAlign_Fill)
							.Padding(PaddingAmount)
							[
								SNew(SStepWidget)
								.StepName(LOCTEXT("FixUpRedirectsStepName", "Fix Up Redirectors"))
								.StepDesc(LOCTEXT("FixUpRedirectsStepDesc", "Resave all packages that point to redirectors in your project, and delete those redirectors if able to resave all the things referencing them."))
								.Icon(FLinterStyle::Get()->GetBrush("Linter.Step.FixUpRedirects.Thumbnail"))
								.ShowStepStatusIcon(false)
								.StepStatus_Lambda([this]() { return FixUpRedirectorStatus; })
								.StepActionText(LOCTEXT("FixUpRedirectsStepAction", "Fix Up Redirectors"))
								.OnPerformAction_Lambda([this](FScopedSlowTask& ScopedSlowTask)
								{
									FixUpRedirectorStatus = EStepStatus::InProgress;
									bool bSuccess = true;

									FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
									ScopedSlowTask.EnterProgressFrame(0, LOCTEXT("Linter.FixUpRedirects.FindingRedirectors", "Looking For redirectors..."));

									// Form a filter from the paths
									FARFilter Filter;
									Filter.bRecursivePaths = true;
									Filter.PackagePaths.Add("/Game");
									Filter.ClassNames.Add("ObjectRedirector");

									// Query for a list of assets in the selected paths
									TArray<FAssetData> AssetList;
									AssetRegistryModule.Get().GetAssets(Filter, AssetList);

									if (AssetList.Num() > 0)
									{
										TArray<FString> ObjectPaths;
										for (const auto& Asset : AssetList)
										{
											ObjectPaths.Add(Asset.ObjectPath.ToString());
										}

										ScopedSlowTask.EnterProgressFrame(0.25f, LOCTEXT("Linter.FixUpRedirects.LoadingRedirectors", "Loading redirectors..."));

										TArray<UObject*> Objects;
										if (LoadAssetsIfNeeded(ObjectPaths, Objects))
										{
											// Transform Objects array to ObjectRedirectors array
											TArray<UObjectRedirector*> Redirectors;
											for (auto Object : Objects)
											{
												auto Redirector = CastChecked<UObjectRedirector>(Object);
												Redirectors.Add(Redirector);
											}

											ScopedSlowTask.EnterProgressFrame(0.25f, LOCTEXT("Linter.FixUpRedirects.LoadingRedirectors", "Fixing up redirectors..."));

											// Load the asset tools module
											const FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
											AssetToolsModule.Get().FixupReferencers(Redirectors);
										}
										else
										{
											FNotificationInfo NotificationInfo(LOCTEXT("FixUpRedirectorsFailed", "Linter failed to load an object redirector when trying to fix up all redirectors."));
											NotificationInfo.ExpireDuration = 6.0f;
											NotificationInfo.Hyperlink = FSimpleDelegate::CreateStatic([]() { FMessageLog("LoadErrors").Open(EMessageSeverity::Info, true); });
											NotificationInfo.HyperlinkText = LOCTEXT("LoadObjectHyperlink", "Show Message Log");
											FSlateNotificationManager::Get().AddNotification(NotificationInfo);
											bSuccess = false;
										}
									}

									FixUpRedirectorStatus = bSuccess ? EStepStatus::Success : EStepStatus::Error;
								})
							]
							// Build Lighting Widget
							+ SVerticalBox::Slot()
							.Padding(PaddingAmount)
							[
								SNew(SBorder)
								.BorderImage(FAppStyle::GetBrush("NoBorder"))
								.Padding(PaddingAmount)
								.Visibility_Lambda([&](){ return (MapAssetDataList.Num() > 0) ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed; })
								[
									SNew(SBorder)
									.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
									.Padding(PaddingAmount)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(SHorizontalBox)
											// Template thumbnail image
											+ SHorizontalBox::Slot()
											.Padding(4.0)
											.AutoWidth()
											.VAlign(VAlign_Top)
											[
												SNew(SImage)
												.Image(FLinterStyle::Get()->GetBrush("Linter.Step.BuildLighting.Thumbnail"))
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
													.Text(LOCTEXT("BuildLightingStepName", "Build Lighting and Run Map Check"))
													.TextStyle(FLinterStyle::Get(), "Linter.Report.RuleTitle")
												]

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(PaddingAmount)
												[
													SNew(SRichTextBlock)
													.Text(LOCTEXT("BuildLightingStepDesc", "Open each map listed below and click Map Check in the Build Options Menu of the Level Editor Toolbar. If any Map Check errors are generated, resolve them before packaging your project. If any 'LIGHTING NEEDS TO BE REBUILT' errors are present press Ctrl+Shift+Semicolon to rebuild lighting."))
													.TextStyle(FLinterStyle::Get(), "Linter.Report.DescriptionText")
													.AutoWrapText(true)
												]
											]
										]
										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(SSeparator)
										]
										+ SVerticalBox::Slot()
										.VAlign(VAlign_Fill)
										.FillHeight(1.0f)
										.Padding(PaddingAmount)
										[
											SAssignNew(MarketplaceRecommendationMapScrollBoxPtr, SScrollBox)
											.ScrollBarAlwaysVisible(true)
										]
									]
								]
							]
							// Save All Step
							+ SVerticalBox::Slot()
							.Padding(PaddingAmount)
							.AutoHeight()
							[
								SNew(SStepWidget)
								.StepName(LOCTEXT("SaveAllStepName", "Save All"))
								.StepDesc(LOCTEXT("SaveAllStepDesc", "Save all unsaved levels and assets to disk."))
								.Icon(FLinterStyle::Get()->GetBrush("Linter.Step.SaveAll.Thumbnail"))
								.ShowStepStatusIcon(false)
								.StepStatus_Lambda([this]() { return SaveAllStatus; })
								.StepActionText(LOCTEXT("SaveAllStepNameAction", "Save All"))
								.OnPerformAction_Lambda([this](FScopedSlowTask& ScopedSlowTask)
								{
									SaveAllStatus = EStepStatus::InProgress;

									// Taken from MainFrameActions.cpp SaveAll
									const bool bPromptUserToSave = false;
									const bool bSaveMapPackages = true;
									const bool bSaveContentPackages = true;
									const bool bFastSave = false;
									const bool bNotifyNoPackagesSaved = false;
									const bool bCanBeDeclined = false;
									if (FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages, bFastSave, bNotifyNoPackagesSaved, bCanBeDeclined))
									{
										SaveAllStatus = EStepStatus::Success;
									}
									else
									{
										FNotificationInfo NotificationInfo(LOCTEXT("SaveAllFailed", "Linter failed to save all dirty assets!"));
										NotificationInfo.ExpireDuration = 6.0f;
										NotificationInfo.Hyperlink = FSimpleDelegate::CreateStatic([]() { FMessageLog("LoadErrors").Open(EMessageSeverity::Info, true); });
										NotificationInfo.HyperlinkText = LOCTEXT("LoadObjectHyperlink", "Show Message Log");
										FSlateNotificationManager::Get().AddNotification(NotificationInfo);
										SaveAllStatus = EStepStatus::Error;
									}					
								})
							]
							// Package Project step
							+ SVerticalBox::Slot()
							.Padding(PaddingAmount)
							.AutoHeight()
							[
								SNew(SBorder)
								.Visibility(EVisibility::Collapsed)
								.BorderImage(FAppStyle::GetBrush("NoBorder"))
								.Padding(PaddingAmount)
								[
									SNew(SBorder)
									.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
									.Padding(PaddingAmount)
									[
										SNew(SVerticalBox)
										+ SVerticalBox::Slot()
										[
											SNew(SHorizontalBox)
											+ SHorizontalBox::Slot()
											.Padding(4.0)
											.AutoWidth()
											.VAlign(VAlign_Top)
											[
												SNew(SImage)
												.Image(FLinterStyle::Get()->GetBrush("Linter.Step.Package.Thumbnail"))
											]
											+ SHorizontalBox::Slot()
											[
												SNew(SVerticalBox)
												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(PaddingAmount)
												[
													SNew(STextBlock)
													.Text(LOCTEXT("PackageProduct", "Package Product to .Zip"))
													.TextStyle(FLinterStyle::Get(), "Linter.Report.RuleTitle")
												]

												+ SVerticalBox::Slot()
												.AutoHeight()
												.Padding(PaddingAmount)
												[
													SNew(SRichTextBlock)
													.Text(LOCTEXT("PackageProductDesc", "Upload this .zip file to a hosting site (GoogleDrive/Dropbox/etc.) and enter the download URL as the associated product's Project File Link in the Publisher Portal."))
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
														.OnClicked_Lambda([&]()
														{
		#if PLATFORM_WINDOWS
															FText PlatformName = LOCTEXT("PlatformName_Windows", "Windows");
		#elif PLATFORM_MAC
															FText PlatformName = LOCTEXT("PlatformName_Mac", "Mac");
		#elif PLATFORM_LINUX
															FText PlatformName = LOCTEXT("PlatformName_Linux", "Linux");
		#else
															FText PlatformName = LOCTEXT("PlatformName_Other", "Other OS");
		#endif

															bool bOpened = false;
															TArray<FString> SaveFilenames;
															IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
															if (DesktopPlatform != NULL)
															{
																bOpened = DesktopPlatform->SaveFileDialog(
																	NULL,
																	NSLOCTEXT("UnrealEd", "ZipUpProject", "Zip file location").ToString(),
																	FPaths::ProjectDir(),
																	FApp::GetProjectName(),
																	TEXT("Zip file|*.zip"),
																	EFileDialogFlags::None,
																	SaveFilenames);
															}

															// We never want to compile editor targets when invoking UAT in this context.
															// If we are installed or don't have a compiler, we must assume we have a precompiled UAT.
															const TCHAR* UATFlags = (FApp::GetEngineIsPromotedBuild() || FApp::IsEngineInstalled())
																? TEXT("-nocompile -nocompileeditor")
																: TEXT("-nocompileeditor");

															if (bOpened)
															{
																for (FString FileName : SaveFilenames)
																{
																	// Ensure path is full rather than relative (for macs)
																	FString FinalFileName = FPaths::ConvertRelativePathToFull(FileName);
																	FString ProjectPath = FPaths::IsProjectFilePathSet() ? FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) : FPaths::RootDir() / FApp::GetProjectName();

																	FString CommandLine = FString::Printf(TEXT("ZipProjectUp %s -project=\"%s\" -install=\"%s\""), UATFlags, *ProjectPath, *FinalFileName);

																	IUATHelperModule::Get().CreateUatTask(CommandLine, PlatformName, LOCTEXT("ZipTaskName", "Zipping Up Project"),
																		LOCTEXT("ZipTaskShortName", "Zip Project Task"), FAppStyle::GetBrush(TEXT("MainFrame.CookContent")));
																}

																FGlobalTabmanager::Get()->FTabManager::TryInvokeTab(FName("LinterTab"))->RequestCloseTab();
															}
															return FReply::Handled();
														})
														[
															SNew(STextBlock)
															.Text(LOCTEXT("SelectOutputFolder", "Package Project into a .zip"))
														]
													]
												]
											]
										]
									]
								]
							]
						]
					]
				]
			]
		]
	];

	// Determine all levels in the project for wizard purposes
	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> AssetDatas;
	FARFilter Filter;
	Filter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(TEXT("/Game"));
	AssetRegistryModule.Get().GetAssets(Filter, AssetDatas);

	MapAssetDataList.Empty();
	for (FAssetData Asset : AssetDatas)
	{
		MapAssetDataList.Add(MakeShareable(new FAssetData(Asset)));

		MarketplaceRecommendationMapScrollBoxPtr.Get()->AddSlot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		.Padding(PaddingAmount)
		[
			SNew(SAssetLinkWidget)
			.AssetData(Asset)
		];
	}
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLintWizard::OnLintReportEntered()
{
	LintReport->Rebuild(CastChecked<ULintRuleSet>(SelectedRuleSet->GetAsset()));
}

void SLintWizard::OnMarketplaceRecommendationsEntered()
{
	bOfferPackage = true;
}

bool SLintWizard::LoadAssetsIfNeeded(const TArray<FString>& ObjectPaths, TArray<UObject*>& LoadedObjects)
{
	bool bAnyObjectsWereLoadedOrUpdated = false;

	// Build a list of unloaded assets
	TArray<FString> UnloadedObjectPaths;
	bool bAtLeastOneUnloadedMap = false;
	for (int32 PathIdx = 0; PathIdx < ObjectPaths.Num(); ++PathIdx)
	{
		const FString& ObjectPath = ObjectPaths[PathIdx];

		UObject* FoundObject = FindObject<UObject>(NULL, *ObjectPath);
		if (FoundObject)
		{
			LoadedObjects.Add(FoundObject);
		}
		else
		{
			// Unloaded asset, we will load it later
			UnloadedObjectPaths.Add(ObjectPath);
			if (FEditorFileUtils::IsMapPackageAsset(ObjectPath))
			{
				bAtLeastOneUnloadedMap = true;
			}
		}
	}

	// Make sure all selected objects are loaded, where possible
	if (UnloadedObjectPaths.Num() > 0)
	{
		FScopedSlowTask SlowTask(UnloadedObjectPaths.Num(), LOCTEXT("LoadingObjects", "Loading Objects..."));
		SlowTask.MakeDialog();

		GIsEditorLoadingPackage = true;

		const ELoadFlags LoadFlags = LOAD_None;
		bool bSomeObjectsFailedToLoad = false;
		for (int32 PathIdx = 0; PathIdx < UnloadedObjectPaths.Num(); ++PathIdx)
		{
			const FString& ObjectPath = UnloadedObjectPaths[PathIdx];
			SlowTask.EnterProgressFrame(1, FText::Format(LOCTEXT("LoadingObjectf", "Loading {0}..."), FText::FromString(ObjectPath)));

			// Load up the object
			UObject* LoadedObject = LoadObject<UObject>(NULL, *ObjectPath, NULL, LoadFlags, NULL);
			if (LoadedObject)
			{
				LoadedObjects.Add(LoadedObject);
			}
			else
			{
				bSomeObjectsFailedToLoad = true;
			}

			if (GWarn->ReceivedUserCancel())
			{
				// If the user has canceled stop loading the remaining objects. We don't add the remaining objects to the failed string,
				// this would only result in launching another dialog when by their actions the user clearly knows not all of the 
				// assets will have been loaded.
				break;
			}
		}
		GIsEditorLoadingPackage = false;

		if (bSomeObjectsFailedToLoad)
		{
			return false;
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE