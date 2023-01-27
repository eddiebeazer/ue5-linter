// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#include "LintRules/LintRule_Path_IsNotTooLong.h"
#include "LintRuleSet.h"
#include "LinterNamingConvention.h"
#include "HAL/FileManager.h"

ULintRule_Path_IsNotTooLong::ULintRule_Path_IsNotTooLong(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool ULintRule_Path_IsNotTooLong::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	FString PathName = ObjectToLint->GetPathName();

	// See if file path is longer than 140 characters
	// 145 = 140 + /Game (5)
	int32 DotIndex = -1;
	PathName.FindLastChar('.', DotIndex);
	FString FilePath = PathName.LeftChop(PathName.Len() - DotIndex);
	if (FilePath.Len() >= MaxPathLimit + 5)
	{
		OutRuleViolations.Push(FLintRuleViolation(ObjectToLint, GetClass()));
		return false;
	}
	
	return true;
}