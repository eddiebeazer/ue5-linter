// Copyright 2016 Gamemakin LLC. All Rights Reserved.

#include "BatchRenameTool/BatchRenameTool.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Text/TextLayout.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Types/SlateEnums.h"
#include "Editor.h"
#include "AssetToolsModule.h"
#include "Modules/ModuleManager.h"
#include "IAssetTools.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Logging/MessageLog.h"
#include "Logging/TokenizedMessage.h"

#define LOCTEXT_NAMESPACE "LinterBatchRenamer"

FDlgBatchRenameTool::FDlgBatchRenameTool(const TArray<FAssetData> Assets)
	: bRemovePrefix(false)
	, bRemoveSuffix(false)
	, SelectedAssets(Assets)
{
	if (FSlateApplication::IsInitialized())
	{
		DialogWindow = SNew(SWindow)
			.Title(LOCTEXT("BatchRenameToolDlgTitle", "Batch Rename Tool"))
			.SupportsMinimize(false).SupportsMaximize(false)
			.SaneWindowPlacement(true)
			.AutoCenter(EAutoCenter::PreferredWorkArea)
			.ClientSize(FVector2D(350, 165));

		TSharedPtr<SBorder> DialogWrapper =
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			.Padding(4.0f)
			[
				SAssignNew(DialogWidget, SDlgBatchRenameTool)
				.ParentWindow(DialogWindow)
			];

		DialogWindow->SetContent(DialogWrapper.ToSharedRef());
	}
}
FDlgBatchRenameTool::EResult FDlgBatchRenameTool::ShowModal()
{
	//Show Dialog
	GEditor->EditorAddModalWindow(DialogWindow.ToSharedRef());
	EResult UserResponse = (EResult)DialogWidget->GetUserResponse();

	if (UserResponse == EResult::Confirm)
	{
		Prefix = DialogWidget->PrefixTextBox->GetText().ToString();
		Suffix = DialogWidget->SuffixTextBox->GetText().ToString();
		bRemovePrefix = DialogWidget->PrefixRemoveBox->IsChecked();
		bRemoveSuffix = DialogWidget->SuffixRemoveBox->IsChecked();

		Find = DialogWidget->FindTextBox->GetText().ToString();
		Replace = DialogWidget->ReplaceTextBox->GetText().ToString();

		// If no information is given, treat as canceled
		if (Prefix.IsEmpty() && Suffix.IsEmpty() && Find.IsEmpty())
		{
			return EResult::Cancel;
		}

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
		TArray<FAssetRenameData> AssetsAndNames;

		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& Asset = *AssetIt;

			// Early out on assets that can not be renamed
			if (!(!Asset.IsRedirector() && Asset.AssetClass != NAME_Class && !(Asset.PackageFlags & PKG_FilterEditorOnly)))
			{
				continue;
			}

			// Work on a copy of the asset name and see if after name operations
			// if the copy is different than the original before creating rename data
			FString AssetNewName = Asset.AssetName.ToString();

			if (!Find.IsEmpty())
			{
				AssetNewName.ReplaceInline(*Find, *Replace);
			}

			if (!Prefix.IsEmpty())
			{
				if (bRemovePrefix)
				{
					AssetNewName.RemoveFromStart(Prefix, ESearchCase::CaseSensitive);
				}
				else
				{
					if (!AssetNewName.StartsWith(Prefix, ESearchCase::CaseSensitive))
					{
						AssetNewName.InsertAt(0, Prefix);
					}
				}
			}

			if (!Suffix.IsEmpty())
			{
				if (bRemoveSuffix)
				{
					AssetNewName.RemoveFromEnd(Suffix, ESearchCase::CaseSensitive);
				}
				else
				{
					if (!AssetNewName.EndsWith(Suffix, ESearchCase::CaseSensitive))
					{
						AssetNewName = AssetNewName.Append(Suffix);
					}
				}
			}

			if (AssetNewName != Asset.AssetName.ToString())
			{
				AssetsAndNames.Push(FAssetRenameData(Asset.GetAsset(), Asset.PackagePath.ToString(), AssetNewName));
			}
		}

		if (!AssetToolsModule.Get().RenameAssets(AssetsAndNames))
		{
			FNotificationInfo NotificationInfo(LOCTEXT("BatchRenameFailed", "Batch Rename operation did not fully complete successfully. Maybe fix up redirectors? Check Output Log for details!"));
			NotificationInfo.ExpireDuration = 6.0f;
			NotificationInfo.Hyperlink = FSimpleDelegate::CreateStatic([]() { FMessageLog("LoadErrors").Open(EMessageSeverity::Info, true); });
			NotificationInfo.HyperlinkText = LOCTEXT("LoadObjectHyperlink", "Show Message Log");
			FSlateNotificationManager::Get().AddNotification(NotificationInfo);
		}
	}
	return UserResponse;
}

void SDlgBatchRenameTool::Construct(const FArguments& InArgs)
{
	UserResponse = FDlgBatchRenameTool::Cancel;
	ParentWindow = InArgs._ParentWindow.Get();

	this->ChildSlot[
		SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 4.0f, 8.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(48.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("BatchRenameToolDlgPrefix", "Prefix"))
					.Justification(ETextJustify::Right)
					]
				]
			+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SAssignNew(PrefixTextBox, SEditableTextBox)
				]
			+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 0.0f, 0.0f)
				[
					SAssignNew(PrefixRemoveBox, SCheckBox)
				]
			+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BatchRenameToolDlgRemove", "Remove"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 4.0f, 8.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Right)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(48.0f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("BatchRenameToolDlgSuffix", "Suffix"))
					.Justification(ETextJustify::Right)
					]
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SAssignNew(SuffixTextBox, SEditableTextBox)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 0.0f, 0.0f)
				[
					SAssignNew(SuffixRemoveBox, SCheckBox)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BatchRenameToolDlgRemove", "Remove"))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 4.0f, 8.0f, 4.0f)
			[
				SNew(SSeparator)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(8.0f, 4.0f, 8.0f, 4.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(48.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BatchRenameToolDlgFind", "Find"))
					.Justification(ETextJustify::Right)
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SAssignNew(FindTextBox, SEditableTextBox)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(48.0f)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("BatchRenameToolDlgReplace", "Replace"))
					.Justification(ETextJustify::Right)
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SAssignNew(ReplaceTextBox, SEditableTextBox)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 4.0f)
		[
			SNew(SSeparator)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Right)
		.Padding(8.0f, 4.0f, 8.0f, 4.0f)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
			.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SDlgBatchRenameTool::OnButtonClick, FDlgBatchRenameTool::Confirm)
				.Text(LOCTEXT("SkeletonMergeOk", "OK"))
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.OnClicked(this, &SDlgBatchRenameTool::OnButtonClick, FDlgBatchRenameTool::Cancel)
				.Text(LOCTEXT("SkeletonMergeCancel", "Cancel"))
			]
		]
	];
}

FDlgBatchRenameTool::EResult SDlgBatchRenameTool::GetUserResponse() const
{
	return UserResponse;
}

#undef LOCTEXT_NAMESPACE
