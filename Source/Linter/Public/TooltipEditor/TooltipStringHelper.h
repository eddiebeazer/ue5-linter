#pragma once

#include "Internationalization/Regex.h"
#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "UObject/Object.h"

/**
* Helper struct for showing function tooltip widgets
**/
struct FBPFunctionArgumentDescription
{
	FText ArgumentName;
	FText Tooltip;
	FText ArgumentType;

	FBPFunctionArgumentDescription()
	{
	}

	FBPFunctionArgumentDescription(FText InArgumentName, FText InTooltip, FText InArgumentType = FText::GetEmpty())
		: ArgumentName(InArgumentName)
		, Tooltip(InTooltip)
		, ArgumentType(InArgumentType)
	{
	}

	FText GetToolTipTextRef()
	{
		return Tooltip;
	}
};

class FTooltipStringHelper
{
public:
	static FText ParseFunctionRawTooltipGetDescription(FString RawTooltip, bool bRemoveNewlines = false);
	static bool ParseFunctionRawTooltip(FString RawTooltip, FText& OutFunctionDescription, TArray<TSharedPtr<FBPFunctionArgumentDescription>>& Inputs, TArray<TSharedPtr<FBPFunctionArgumentDescription>>& Outputs, FText& OutReturnText);
	static FString ConvertTooltipDataToRawTooltip(FText FunctionDescription, TArray<TSharedPtr<FBPFunctionArgumentDescription>> Inputs, TArray<TSharedPtr<FBPFunctionArgumentDescription>> Outputs);
	static bool FindAndUpdateArgumentTooltip(FText ArgumentName, FText Tooltip, TArray<TSharedPtr<FBPFunctionArgumentDescription>>& Arguments);
};