// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LintRule.generated.h"

UENUM(BlueprintType)
enum class ELintRuleSeverity : uint8
{
	Error,
	Warning,
	Info
//	Ignore
};

USTRUCT(BlueprintType)
struct LINTER_API FLintRuleViolation
{
	GENERATED_USTRUCT_BODY()

	FLintRuleViolation()
		: Violator(nullptr)
		, ViolatedRule(nullptr)
		, RecommendedAction(FText::GetEmpty())
	{
	}

	FLintRuleViolation(UObject* InViolator, TSubclassOf<ULintRule> InViolatedRule, const FText InRecommendedAction = FText::GetEmpty())
		: Violator(InViolator)
		, ViolatedRule(InViolatedRule)
		, RecommendedAction(InRecommendedAction)
	{
	}

	// I don't particularly like this way of extracting relevant data, but alas here we are.
	static TArray<FLintRuleViolation> AllRuleViolationsWithViolator(const TArray<FLintRuleViolation>& RuleViolationCollection, const UObject* SearchViolator);
	static TArray<TSharedPtr<FLintRuleViolation>> AllRuleViolationsWithViolatorShared(const TArray<FLintRuleViolation>& RuleViolationCollection, const UObject* SearchViolator);
	static TArray<TSharedPtr<FLintRuleViolation>> AllRuleViolationsWithViolatorShared(const TArray<TSharedPtr<FLintRuleViolation>>& RuleViolationCollection, const UObject* SearchViolator);
	static TArray<FLintRuleViolation> AllRuleViolationsOfSpecificRule(const TArray<FLintRuleViolation>& RuleViolationCollection, TSubclassOf<ULintRule> SearchRule);
	static TArray<FLintRuleViolation> AllRuleViolationsOfRuleGroup(const TArray<FLintRuleViolation>& RuleViolationCollection, FName SearchRuleGroup);

	static TArray<UObject*>  AllRuleViolationViolators(const TArray<FLintRuleViolation>& RuleViolationCollection);
	static TArray<UObject*>  AllRuleViolationViolators(const TArray<TSharedPtr<FLintRuleViolation>>& RuleViolationCollection);
	static TMultiMap<UObject*, FLintRuleViolation>  AllRuleViolationsMappedByViolator(const TArray<FLintRuleViolation>& RuleViolationCollection);
	static TMultiMap<ULintRule*, FLintRuleViolation> AllRuleViolationsMappedByViolatedLintRule(const TArray<FLintRuleViolation>& RuleViolationCollection);
	static TMultiMap<const ULintRule*, TSharedPtr<FLintRuleViolation>> AllRuleViolationsMappedByViolatedLintRuleShared(const TArray<FLintRuleViolation>& RuleViolationCollection);
	static TMultiMap<const ULintRule*, TSharedPtr<FLintRuleViolation>> AllRuleViolationsMappedByViolatedLintRuleShared(const TArray<TSharedPtr<FLintRuleViolation>>& RuleViolationCollection);

	bool PopulateAssetData();

	UPROPERTY(EditAnywhere, Category = "Lint")
	TWeakObjectPtr<UObject> Violator;

	UPROPERTY(EditAnywhere, Category = "Lint")
	TSubclassOf<ULintRule> ViolatedRule;

	UPROPERTY(EditAnywhere, Category = "Lint")
	FText RecommendedAction;

	FAssetData ViolatorAssetData;
};

/**
 *Comment
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class LINTER_API ULintRule : public UObject
{
	GENERATED_BODY()

public:

	ULintRule(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FName RuleGroup;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FText RuleTitle;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FText RuleDescription;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FString RuleURL;

	UPROPERTY(EditDefaultsOnly, Category = "Display")
	ELintRuleSeverity RuleSeverity;

	UPROPERTY(EditDefaultsOnly, Category = "Settings", AdvancedDisplay)
	bool bRequiresGameThread = false;

	UFUNCTION(BlueprintCallable, Category = "Lint")
	virtual bool PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const;
	
	UFUNCTION(BlueprintCallable, Category = "Display")
	virtual bool IsRuleSuppressed() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Display")
	FName GetRuleBasedObjectVariantName(UObject* ObjectToLint) const;

protected:

	/* This is the function that child lint rules should override to perform the meat of the rule check
	 * You do not call this directly. Always call PassesRule. PassesRule forwards to the PassesRule_Internal ONLY IF
	 * data is valid and the rule is not suppressed, therefore it is worth checking.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Lint")
	bool PassesRule_Internal(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const;

private:

};
