// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Misc/ScopedSlowTask.h"
#include "LintRule.h"

#include "LintRuleSet.generated.h"

class ULinterNamingConvention;

USTRUCT(BlueprintType)
struct LINTER_API FLintRuleList
{
	GENERATED_USTRUCT_BODY()

	FLintRuleList()
	{}

	UPROPERTY(EditAnywhere, Category = Default)
	TArray<TSubclassOf<ULintRule>> LintRules;

	bool RequiresGameThread() const;;
	bool PassesRules(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const;
};

/**
 *Comment
 */
UCLASS(BlueprintType, Blueprintable)
class LINTER_API ULintRuleSet : public UDataAsset
{
	GENERATED_BODY()

public:
	ULintRuleSet(const FObjectInitializer& ObjectInitializer);

	//UFUNCTION(BlueprintCallable, Category = "Conventions")
	const FLintRuleList* GetLintRuleListForClass(TSoftClassPtr<UObject> Class) const;

	UFUNCTION(BlueprintCallable, Category = "Conventions")
	ULinterNamingConvention* GetNamingConvention() const;

	/** Invoke this with a list of asset paths to recursively lint all assets in paths. */
	//UFUNCTION(BlueprintCallable, Category = "Lint")
	TArray<FLintRuleViolation> LintPath(TArray<FString> AssetPaths, FScopedSlowTask* ParentScopedSlowTask = nullptr) const;

	/** This is a temp dumb way to do this. */
	TArray<TSharedPtr<FLintRuleViolation>> LintPathShared(TArray<FString> AssetPaths, FScopedSlowTask* ParentScopedSlowTask = nullptr) const;

	UPROPERTY(EditDefaultsOnly, Category = "Marketplace")
	bool bShowMarketplacePublishingInfoInLintWizard = false;

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	FText RuleSetDescription;

	UPROPERTY(EditDefaultsOnly, Category = "Commandlet")
	FString NameForCommandlet;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	TSoftObjectPtr<ULinterNamingConvention> NamingConvention;

	UPROPERTY(EditDefaultsOnly, Category = "Rules")
	TMap<TSubclassOf<UObject>, FLintRuleList> ClassLintRulesMap;

};

