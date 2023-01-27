// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeCounter.h"
#include "HAL/Runnable.h"
#include "AssetData.h"
#include "Linter.h"

class FLintRunner : public FRunnable
{

public:

	FLintRunner(UObject* InLoadedObject, const ULintRuleSet* LintRuleSet, TArray<FLintRuleViolation>* InpOutRuleViolations, FScopedSlowTask* InParentScopedSlowTask);

	virtual bool RequiresGamethread();

	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

protected:
	UObject* LoadedObject = nullptr;
	const ULintRuleSet* RuleSet = nullptr;
	TArray<FLintRuleViolation>* pOutRuleViolations;

	const FLintRuleList* pLoadedRuleList;
	static FCriticalSection LintDataUpdateLock;

	FScopedSlowTask* ParentScopedSlowTask;
};

