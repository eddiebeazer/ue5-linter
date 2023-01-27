#include "TooltipEditor/TooltipStringHelper.h"

FText FTooltipStringHelper::ParseFunctionRawTooltipGetDescription(FString RawTooltip, bool bRemoveNewlines/* = false*/)
{
	if (RawTooltip.Len() == 0)
	{
		return FText::GetEmpty();
	}

	TArray<FString> Lines;
	RawTooltip.ParseIntoArrayLines(Lines);

	FString Description;

	for (FString Line : Lines)
	{
		if (Line.StartsWith(TEXT("@param")))
		{
			break;
		}
		else if (Line.StartsWith(TEXT("@return"))) //@return is assumed to always be last
		{
			break;
		}
		else
		{
			if (Description.Len() > 0)
			{
				Description.Append(TEXT("\n"));
			}
			Description.Append(Line);
		}
	}

	if (bRemoveNewlines)
	{
		Description.ReplaceInline(TEXT("\r"), TEXT(""));
		Description.ReplaceInline(TEXT("\n"), TEXT(" "));
	}

	return FText::FromString(Description);
}

bool FTooltipStringHelper::ParseFunctionRawTooltip(FString RawTooltip, FText& OutFunctionDescription, TArray<TSharedPtr<FBPFunctionArgumentDescription>>& Inputs, TArray<TSharedPtr<FBPFunctionArgumentDescription>>& Outputs, FText& OutReturnText)
{
	if (RawTooltip.Len() == 0)
	{
		return false;
	}

	TArray<FString> Lines;
	RawTooltip.ParseIntoArrayLines(Lines);

	FString Description;
	bool bParsingDescription = true;

	FText CurrentArgumentName;
	FString CurrentArgumentTooltip;
	bool bCurrentArgumentIsInput = true;
	
	for (FString Line : Lines)
	{
		if (Line.StartsWith(TEXT("@param")))
		{
			if (!bParsingDescription)
			{
				if (bCurrentArgumentIsInput)
				{
					FTooltipStringHelper::FindAndUpdateArgumentTooltip(CurrentArgumentName, FText::FromString(CurrentArgumentTooltip), Inputs);
				}
				else
				{
					FTooltipStringHelper::FindAndUpdateArgumentTooltip(CurrentArgumentName, FText::FromString(CurrentArgumentTooltip), Outputs);
				}

				CurrentArgumentName = FText::GetEmpty();
				CurrentArgumentTooltip.Empty();
			}

			bParsingDescription = false;
			bCurrentArgumentIsInput = true;

			Line = Line.RightChop(6);
			Line.TrimStartAndEndInline();
			
			if (Line.StartsWith("[out]"))
			{
				bCurrentArgumentIsInput = false;
				Line = Line.RightChop(5);
				Line.TrimStartAndEndInline();
			}

			Line = Line.ConvertTabsToSpaces(4);
			FString ArgumentName;
			FString ArgumentTooltip;
			if (Line.Split(TEXT(" "), &ArgumentName, &CurrentArgumentTooltip))
			{
				CurrentArgumentName = FText::FromString(ArgumentName);
				CurrentArgumentTooltip.TrimStartAndEndInline();
			}			
		}
		else if (Line.StartsWith(TEXT("@return"))) //@return is assumed to always be last
		{
			if (!bParsingDescription)
			{
				if (bCurrentArgumentIsInput)
				{
					FTooltipStringHelper::FindAndUpdateArgumentTooltip(CurrentArgumentName, FText::FromString(CurrentArgumentTooltip), Inputs);
				}
				else
				{
					FTooltipStringHelper::FindAndUpdateArgumentTooltip(CurrentArgumentName, FText::FromString(CurrentArgumentTooltip), Outputs);
				}

				CurrentArgumentName = FText::GetEmpty();
				CurrentArgumentTooltip.Empty();
			}

			Line = Line.RightChop(7);
			Line.TrimStartAndEndInline();
			OutReturnText = FText::FromString(Line);
		}
		else
		{
			if (bParsingDescription)
			{
				if (Description.Len() > 0)
				{
					Description.Append(TEXT("\n"));
				}
				Description.Append(Line);
			}
			else
			{
				Line.TrimStartAndEndInline();
				CurrentArgumentTooltip.AppendChar(TEXT(' '));
				CurrentArgumentTooltip.Append(Line);
			}
		}
	}

	if (!CurrentArgumentName.IsEmpty())
	{
		if (!bParsingDescription)
		{
			if (bCurrentArgumentIsInput)
			{
				FTooltipStringHelper::FindAndUpdateArgumentTooltip(CurrentArgumentName, FText::FromString(CurrentArgumentTooltip), Inputs);
			}
			else
			{
				FTooltipStringHelper::FindAndUpdateArgumentTooltip(CurrentArgumentName, FText::FromString(CurrentArgumentTooltip), Outputs);
			}

			CurrentArgumentName = FText::GetEmpty();
			CurrentArgumentTooltip.Empty();
		}
	}

	OutFunctionDescription = FText::FromString(Description);
	return true;
}

FString FTooltipStringHelper::ConvertTooltipDataToRawTooltip(FText FunctionDescription, TArray<TSharedPtr<FBPFunctionArgumentDescription>> Inputs, TArray<TSharedPtr<FBPFunctionArgumentDescription>> Outputs)
{
	FString RawTooltip = FunctionDescription.ToString();
	for (TSharedPtr<FBPFunctionArgumentDescription> Arg : Inputs)
	{
		RawTooltip.Append(FString::Printf(TEXT("\n@param %s  %s\t\t\t%s"), TEXT("     "), *Arg->ArgumentName.ToString(), *Arg->Tooltip.ToString()));		
	}

	if (Outputs.Num() == 1)
	{
		RawTooltip.Append(FString::Printf(TEXT("\n@return       %s"), *Outputs[0]->Tooltip.ToString()));
	}
	else
	{
		for (TSharedPtr<FBPFunctionArgumentDescription> Arg : Outputs)
		{
			RawTooltip.Append(FString::Printf(TEXT("\n@param %s  %s\t\t\t%s"), TEXT("[out]"), *Arg->ArgumentName.ToString(), *Arg->Tooltip.ToString()));
		}
	}	

	return RawTooltip;
}

bool FTooltipStringHelper::FindAndUpdateArgumentTooltip(FText ArgumentName, FText Tooltip, TArray<TSharedPtr<FBPFunctionArgumentDescription>>& Arguments)
{
	TSharedPtr<FBPFunctionArgumentDescription>* FuncArg = Arguments.FindByPredicate([&](TSharedPtr<FBPFunctionArgumentDescription> Arg){ return Arg->ArgumentName.EqualTo(ArgumentName, ETextComparisonLevel::Quinary);});
	if (FuncArg != nullptr)
	{
		(*FuncArg)->Tooltip = Tooltip;
		return true;
	}
	else
	{
		return false;
	}
}
