// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#include "LintRunner.h"

#define LOCTEXT_NAMESPACE "Linter"

FCriticalSection FLintRunner::LintDataUpdateLock;

FLintRunner::FLintRunner(UObject* InLoadedObject, const ULintRuleSet* LintRuleSet, TArray<FLintRuleViolation>* InpOutRuleViolations, FScopedSlowTask* InParentScopedSlowTask)
	: LoadedObject(InLoadedObject)
	, RuleSet(LintRuleSet)
	, pOutRuleViolations(InpOutRuleViolations)
	, pLoadedRuleList(LintRuleSet != nullptr ? LintRuleSet->GetLintRuleListForClass(InLoadedObject->GetClass()) : nullptr)
	, ParentScopedSlowTask(InParentScopedSlowTask)
{
}

bool FLintRunner::RequiresGamethread()
{
	if (pLoadedRuleList != nullptr)
	{
		return pLoadedRuleList->RequiresGameThread();
	}

	return false;
}

bool FLintRunner::Init()
{
	if (LoadedObject == nullptr)
	{
		return false;
	}

	if (RuleSet == nullptr)
	{
		return false;
	}

	if (pLoadedRuleList == nullptr)
	{
		return false;
	}

	if (pOutRuleViolations == nullptr)
	{
		return false;
	}

	return true;
}

uint32 FLintRunner::Run()
{	
	if (LoadedObject == nullptr || pLoadedRuleList == nullptr || RuleSet == nullptr || pOutRuleViolations == nullptr)
	{
		return 2;
	}

	FString const AssetPath = LoadedObject->GetPathName();
	UE_LOG(LogLinter, Display, TEXT("Loaded '%s'..."), *AssetPath);

	TArray<FLintRuleViolation> RuleViolations;
	pLoadedRuleList->PassesRules(LoadedObject, RuleSet, RuleViolations);

	if (RuleViolations.Num() > 0)
	{
		FScopeLock lock(&LintDataUpdateLock);
		pOutRuleViolations->Append(RuleViolations);
	}

	UE_LOG(LogLinter, Display, TEXT("Finished '%s'..."), *AssetPath);
	return 0;
}

void FLintRunner::Stop()
{
	return; // this runner doesn't have anything on-going so there is nothing to stop
}

void FLintRunner::Exit()
{
	return;
}

#undef LOCTEXT_NAMESPACE
