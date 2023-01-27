// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "LinterStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "EditorStyleSet.h"

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FLinterStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(StyleSet->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelativePath, ...) FSlateFontInfo(StyleSet->RootToContentDir(RelativePath, TEXT(".ttf")), __VA_ARGS__)
#define TTF_CORE_FONT(RelativePath, ...) FSlateFontInfo(StyleSet->RootToCoreContentDir(RelativePath, TEXT(".ttf") ), __VA_ARGS__)

FString FLinterStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Linter"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

TSharedPtr< FSlateStyleSet > FLinterStyle::StyleSet = nullptr;
TSharedPtr< class ISlateStyle > FLinterStyle::Get() { return StyleSet; }

FName FLinterStyle::GetStyleSetName()
{
	static FName LinterStyleName(TEXT("LinterStyle"));
	return LinterStyleName;
}

void FLinterStyle::Initialize()
{
	// Const icon sizes
	const FVector2D Icon8x8(8.0f, 8.0f);
	const FVector2D Icon14x14(14.0f, 14.0f);
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon64x64(64.0f, 64.0f);
	const FVector2D Icon128x128(128.0f, 128.0f);

	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	// Asset actions
	{
		StyleSet->Set("AssetActions.RunLinter", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_Texture_Run_16x"), Icon16x16));
		StyleSet->Set("AssetActions.BatchRename", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_Texture_Run_16x"), Icon16x16));
		StyleSet->Set("AssetActions.TooltipTool", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_Texture_Run_16x"), Icon16x16));

		// Toolbar Button Icons
		StyleSet->Set("Linter.Toolbar.Icon", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_Texture_Run_16x"), Icon16x16));

		// Report Images
		StyleSet->Set("Linter.Step.Unknown", new IMAGE_PLUGIN_BRUSH("Icons/CompileStatus_Unknown_64px", Icon64x64));
		StyleSet->Set("Linter.Step.Error", new IMAGE_PLUGIN_BRUSH("Icons/CompileStatus_Fail_64px", Icon64x64));
		StyleSet->Set("Linter.Step.Good", new IMAGE_PLUGIN_BRUSH("Icons/CompileStatus_Good_64px", Icon64x64));
		StyleSet->Set("Linter.Step.Working", new IMAGE_PLUGIN_BRUSH("Icons/CompileStatus_Working_64px", Icon64x64));
		StyleSet->Set("Linter.Step.Warning", new IMAGE_PLUGIN_BRUSH("Icons/CompileStatus_Warning_64px", Icon64x64));

		StyleSet->Set("Linter.Report.Link", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_Help_Documentation_16x"), Icon16x16));
		StyleSet->Set("Linter.Report.Warning", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_MessageLog_Warning_14x"), Icon14x14));
		StyleSet->Set("Linter.Report.Error", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_MessageLog_Error_14x"), Icon14x14));

		StyleSet->Set("Linter.Report.Info", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_MessageLog_Info_14x"), Icon20x20));


		StyleSet->Set("Linter.Step.BuildLighting.Thumbnail", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_MapCheck_64x"), Icon64x64));
		StyleSet->Set("Linter.Step.Package.Thumbnail", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_AddContent_64x"), Icon64x64));
		StyleSet->Set("Linter.Step.SaveAll.Thumbnail", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_file_saveall_64x"), Icon64x64));
		StyleSet->Set("Linter.Step.FixupRedirects.Thumbnail", new IMAGE_PLUGIN_BRUSH(TEXT("Icons/icon_tab_Toolbars_64x"), Icon64x64));

		// PaCK Sizing
		StyleSet->Set("Linter.Padding",  2.0f);

		// PaCK Fonts
		const FTextBlockStyle NormalText = FEditorStyle::GetWidgetStyle<FTextBlockStyle>("NormalText");

		FTextBlockStyle NameText = FTextBlockStyle(NormalText)
			.SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f));
		{
			NameText.Font.Size = 14;
			StyleSet->Set("Linter.Report.AssetName", NameText);
		}

		FTextBlockStyle RuleTitleText = FTextBlockStyle(NormalText)
			.SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f));
		{
			RuleTitleText.Font.Size = 12;
			StyleSet->Set("Linter.Report.RuleTitle", RuleTitleText);
		}

		FTextBlockStyle DescriptionText = FTextBlockStyle(NormalText)
			.SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f));
		{
			DescriptionText.Font.Size = 10;
			StyleSet->Set("Linter.Report.DescriptionText", DescriptionText);
		}
	}

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};

#undef IMAGE_PLUGIN_BRUSH
#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef TTF_FONT
#undef TTF_CORE_FONT

void FLinterStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}
