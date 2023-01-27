// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LintRuleSet.h"
#include "LinterSettings.generated.h"


/**
* Implements the settings for the Linter plugin.
*/
UCLASS(config = Linter, defaultconfig)
class ULinterSettings : public UObject
{
	GENERATED_UCLASS_BODY()
	
public:

	UPROPERTY(EditAnywhere, config, Category = Settings)
	TSoftObjectPtr<ULintRuleSet> DefaultLintRuleSet;

};
