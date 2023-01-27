// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#include "UI/SAssetLinkWidget.h"

#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Text/SRichTextBlock.h"

void SAssetLinkWidget::Construct(const FArguments& Args)
{
	const float PaddingAmount = FLinterStyle::Get()->GetFloat("Linter.Padding");
	AssetData = Args._AssetData;

	ChildSlot
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SHyperlink)
			.Text(FText::FromName(AssetData.Get().AssetName))
			.OnNavigate_Lambda([&]()
			{
				FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
				FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
				TArray<FAssetData> AssetDatas;
				AssetDatas.Push(AssetRegistryModule.Get().GetAssetByObjectPath(AssetData.Get().ObjectPath));
				ContentBrowserModule.Get().SyncBrowserToAssets(AssetDatas);
			})
		]
	];
}

