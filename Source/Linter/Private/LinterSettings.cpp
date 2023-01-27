// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "LinterSettings.h"
#include "UObject/ConstructorHelpers.h"
#include "LintRuleSet.h"


ULinterSettings::ULinterSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (DefaultLintRuleSet.IsNull())
	{
		static ConstructorHelpers::FObjectFinder<ULintRuleSet> DefaultMarketplaceRuleSetRef(TEXT("LintRuleSet'/Linter/MarketplaceLinter/MarketplaceLintRuleSet.MarketplaceLintRuleSet'"));
		DefaultLintRuleSet = DefaultMarketplaceRuleSetRef.Object;
	}
}