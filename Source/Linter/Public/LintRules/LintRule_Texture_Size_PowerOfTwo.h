// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.h"
#include "Engine/TextureDefines.h"

#include "LintRule_Texture_Size_PowerOfTwo.generated.h"

UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule_Texture_Size_PowerOfTwo : public ULintRule
{
	GENERATED_BODY()

public:
	ULintRule_Texture_Size_PowerOfTwo(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TSet<TEnumAsByte<TextureGroup>> IgnoreTexturesInTheseGroups;

	virtual bool PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

protected:
	virtual bool PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const override;

};
