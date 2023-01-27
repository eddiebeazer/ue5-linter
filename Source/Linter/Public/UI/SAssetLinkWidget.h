// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "EditorStyleSet.h"
#include "LinterStyle.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "Types/SlateStructs.h"
#include "Misc/ScopedSlowTask.h"

class SAssetLinkWidget : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SAssetLinkWidget)
	{
	}

	SLATE_ATTRIBUTE(FAssetData, AssetData)

	SLATE_END_ARGS()

	TAttribute<FAssetData> AssetData;

public:

	void Construct(const FArguments& Args);
};