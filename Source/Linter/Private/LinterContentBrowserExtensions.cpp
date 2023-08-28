// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#include "LinterContentBrowserExtensions.h"
#include "Modules/ModuleManager.h"
#include "LevelEditor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "LinterStyle.h"
#include "LauncherPlatformModule.h"
#include "UnrealEdMisc.h"
#include "ContentBrowserModule.h"
#include "EditorStyleSet.h"
#include "Linter.h"
#include "BatchRenameTool/BatchRenameTool.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/Commands/UIAction.h"
#include "Delegates/IDelegateInstance.h"
#include "TooltipEditor/TooltipTool.h"

#define LOCTEXT_NAMESPACE "Linter"
DEFINE_LOG_CATEGORY_STATIC(LinterContentBrowserExtensions, Log, All);

void FLinterContentBrowserExtensions::InstallHooks(FLinterModule* LinterModule, FDelegateHandle* pContentBrowserExtenderDelegateHandle, class FDelegateHandle* pAssetExtenderDelegateHandle)
{
	struct Local
	{
		// Path extensions

		static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FString>& SelectedPaths)
		{
			TSharedRef<FExtender> Extender = MakeShared<FExtender>();
			Extender->AddMenuExtension(
				"PathContextBulkOperations",
				EExtensionHook::After,
				TSharedPtr<FUICommandList>(),
				FMenuExtensionDelegate::CreateStatic(&Local::ContentBrowserExtenderFunc, SelectedPaths)
			);
			return Extender;
		}

		static void ContentBrowserExtenderFunc(FMenuBuilder& MenuBuilder, const TArray<FString> SelectedPaths)
		{
			MenuBuilder.BeginSection("LinterContentBrowserContext", LOCTEXT("CB_LinterHeader", "Linter"));
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("CB_ScanProjectWithLinter", "Scan with Linter"),
					LOCTEXT("CB_ScanProjectWithLinter_Tooltip", "Scan project content with Linter"),
					FSlateIcon(FLinterStyle::GetStyleSetName(), "Linter.Toolbar.Icon"),
					FUIAction(FExecuteAction::CreateLambda([SelectedPaths]()
					{
						if (FLinterModule* lm = FModuleManager::GetModulePtr<FLinterModule>("Linter"))
						{
							if (lm != nullptr)
							{
								lm->SetDesiredLintPaths(SelectedPaths);
							}
							FGlobalTabmanager::Get()->FTabManager::TryInvokeTab(FName("LinterTab"));
						}
					})),
					NAME_None,
					EUserInterfaceActionType::Button);
			}
			MenuBuilder.EndSection();
		}

		// Asset extensions

		static TSharedRef<FExtender> OnExtendAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
		{
			TSharedRef<FExtender> Extender = MakeShared<FExtender>();
			Extender->AddMenuExtension(
				"CommonAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&Local::AssetExtenderFunc, SelectedAssets)
			);
			return Extender;
		}

		static void AssetExtenderFunc(FMenuBuilder& MenuBuilder, const TArray<FAssetData> SelectedAssets)
		{
			MenuBuilder.BeginSection("LinterAssetContext", LOCTEXT("CB_LinterHeader", "Linter"));
			{
				
				// Run through the assets to determine if any are blueprints
				bool bAnyBlueprintsSelected = false;
				for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
				{
					const FAssetData& Asset = *AssetIt;
					// Cannot rename redirectors or classes or cooked packages
					if (!Asset.IsRedirector() && Asset.AssetClass != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
					{
						if (Asset.GetClass()->IsChildOf(UBlueprint::StaticClass()))
						{
							bAnyBlueprintsSelected = true;
							break;
						}
					}
				}

				// If we have blueprints selected, enable blueprint tools
				if (bAnyBlueprintsSelected)
				{
					// Add Tooltip Editor
					MenuBuilder.AddMenuEntry(
						LOCTEXT("CB_EditBlueprintTooltips", "Edit Blueprint Tooltips (Experimental)"),
						LOCTEXT("CB_EditBlueprintTooltips_Tooltip", "Edit selected blueprints' templates definitions"),
						FSlateIcon(FLinterStyle::GetStyleSetName(), "Linter.Toolbar.Icon"),
						FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
						{
							UE_LOG(LinterContentBrowserExtensions, Display, TEXT("Opening Tooltip Tool window."));
							FTooltipTool AssetDlg(SelectedAssets);
							if (AssetDlg.ShowModal() == FTooltipTool::Confirm)
							{
								UE_LOG(LinterContentBrowserExtensions, Display, TEXT("Tooltip Tool did the thing."));
							}
						})),
						NAME_None,
						EUserInterfaceActionType::Button);
				}

				// Run through the assets to see if any can be renamed
				bool bAnyAssetCanBeRenamed = false;
				for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
				{
					const FAssetData& Asset = *AssetIt;
					// Cannot rename redirectors or classes or cooked packages
					if (!Asset.IsRedirector() && Asset.AssetClass != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly))
					{
						bAnyAssetCanBeRenamed = true;
						break;
					}
				}

				if (bAnyAssetCanBeRenamed)
				{
					// Add Tooltip Editor
					MenuBuilder.AddMenuEntry(
						LOCTEXT("CB_BatchRenameAssets", "Batch Rename Assets (Experimental)"),
						LOCTEXT("CB_BatchRenameAssets_Tooltip", "Perform a bulk rename operation on all of the selected assets"),
						FSlateIcon(FLinterStyle::GetStyleSetName(), "Linter.Toolbar.Icon"),
						FUIAction(FExecuteAction::CreateLambda([SelectedAssets]()
						{
							UE_LOG(LinterContentBrowserExtensions, Display, TEXT("Starting batch rename."));
							FDlgBatchRenameTool AssetDlg(SelectedAssets);
							AssetDlg.ShowModal();
						})),
						NAME_None,
						EUserInterfaceActionType::Button);
				}
			}
			MenuBuilder.EndSection();
		}
	};

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	// Path view extenders
	TArray<FContentBrowserMenuExtender_SelectedPaths>& CBMenuPathExtenderDelegates = ContentBrowserModule.GetAllPathViewContextMenuExtenders();
	CBMenuPathExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedPaths::CreateStatic(&Local::OnExtendContentBrowserAssetSelectionMenu));
	*pContentBrowserExtenderDelegateHandle = CBMenuPathExtenderDelegates.Last().GetHandle();

	// Asset extenders
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuAssetExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&Local::OnExtendAssetSelectionMenu));
	*pAssetExtenderDelegateHandle = CBMenuAssetExtenderDelegates.Last().GetHandle();
}

void FLinterContentBrowserExtensions::RemoveHooks(FLinterModule* LinterModule, FDelegateHandle* pContentBrowserExtenderDelegateHandle, FDelegateHandle* pAssetExtenderDelegateHandle)
{
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

		// Path view extenders
		TArray<FContentBrowserMenuExtender_SelectedPaths>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetContextMenuExtenders();
		CBMenuExtenderDelegates.RemoveAll([pContentBrowserExtenderDelegateHandle](const FContentBrowserMenuExtender_SelectedPaths & Delegate) { return Delegate.GetHandle() == *pContentBrowserExtenderDelegateHandle; });

		// Asset extenders
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuAssetExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
		CBMenuAssetExtenderDelegates.RemoveAll([pAssetExtenderDelegateHandle](const FContentBrowserMenuExtender_SelectedAssets & Delegate) { return Delegate.GetHandle() == *pAssetExtenderDelegateHandle; });
	}
}

#undef LOCTEXT_NAMESPACE