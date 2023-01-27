// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.h"

#include "LintRule_Path_IsNotTooLong.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule_Path_IsNotTooLong : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_Path_IsNotTooLong(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	int32 MaxPathLimit = 140;

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

};
