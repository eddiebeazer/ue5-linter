// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#pragma once
#include "CoreTypes.h"
#include "Styling/ISlateStyle.h"

class FLinterStyle
{
public:
	static void Initialize();

	static void Shutdown();

	static TSharedPtr< class ISlateStyle > Get();
	static TSharedPtr< class FSlateStyleSet > StyleSet;

	static FName GetStyleSetName();
private:
	static FString InContent(const FString& RelativePath, const ANSICHAR* Extension);
};