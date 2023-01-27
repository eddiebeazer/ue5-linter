// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.h"

#include "LintRule_Path_NoTopLevel.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule_Path_NoTopLevel : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_Path_NoTopLevel(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FText ZeroTopLevelFoldersRecommendedAction;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FText PleaseUseThisFolderRecommendedAction;

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

};
