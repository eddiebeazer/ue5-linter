#include "LintRule.h"

#include "Runtime/Launch/Resources/Version.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Blueprint.h"
#include "AssetData.h"
#include "Modules/ModuleManager.h"
#include "IAssetRegistry.h"
#include "IAssetTools.h"
#include "AssetRegistryModule.h"
#include "MaterialDomain.h"


ULintRule::ULintRule(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

bool ULintRule::PassesRule_Internal_Implementation(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, OUT TArray<FLintRuleViolation>& OutRuleViolations) const
{
	return true;
}

bool ULintRule::PassesRule(UObject* ObjectToLint, const ULintRuleSet* ParentRuleSet, TArray<FLintRuleViolation>& OutRuleViolations) const
{
	OutRuleViolations.Empty();

	if (ObjectToLint == nullptr)
	{
		return true;
	}

	if (ParentRuleSet == nullptr)
	{
		return true;
	}

	if (IsRuleSuppressed())
	{
		return true;
	}

	return PassesRule_Internal(ObjectToLint, ParentRuleSet, OutRuleViolations);
}

bool ULintRule::IsRuleSuppressed() const
{
	return false;
}

FName ULintRule::GetRuleBasedObjectVariantName_Implementation(UObject* ObjectToLint) const
{
	if (ObjectToLint == nullptr)
	{
		return NAME_None;
	}

	{
		UMaterialInterface* MI = Cast<UMaterialInterface>(ObjectToLint);
		if (MI != nullptr)
		{
			TMicRecursionGuard RecursionGuard;

			const UMaterial* Material = MI->GetMaterial_Concurrent(RecursionGuard);
			if (Material != nullptr)
			{
				if (Material->MaterialDomain == EMaterialDomain::MD_PostProcess)
				{
					return "PostProcess";
				}
			}
		}
	}

	{
		UBlueprint* Blueprint = Cast<UBlueprint>(ObjectToLint);
		if (Blueprint != nullptr)
		{
			if (Blueprint->BlueprintType == EBlueprintType::BPTYPE_MacroLibrary)
			{
				return "MacroLibrary";
			}

			if (FBlueprintEditorUtils::IsInterfaceBlueprint(Blueprint))
			{
				return "Interface";
			}

			if (Blueprint->BlueprintType == BPTYPE_FunctionLibrary)
			{
				return "FunctionLibrary";
			}
		}
	}

	return NAME_None;
}

TArray<FLintRuleViolation> FLintRuleViolation::AllRuleViolationsWithViolator(const TArray<FLintRuleViolation>& RuleViolationCollection, const UObject* SearchViolator)
{
	return RuleViolationCollection.FilterByPredicate([SearchViolator](const FLintRuleViolation& RuleViolation)
		{
			if (SearchViolator != nullptr && RuleViolation.Violator.Get() == SearchViolator)
			{
				return true;
			}

			return false;
		});
}

TArray<TSharedPtr<FLintRuleViolation>> FLintRuleViolation::AllRuleViolationsWithViolatorShared(const TArray<TSharedPtr<FLintRuleViolation>>& RuleViolationCollection, const UObject* SearchViolator)
{
	return RuleViolationCollection.FilterByPredicate([SearchViolator](const TSharedPtr<FLintRuleViolation>& RuleViolation)
	{
		if (SearchViolator != nullptr && RuleViolation->Violator.Get() == SearchViolator)
		{
			return true;
		}

		return false;
	});
}

TArray<TSharedPtr<FLintRuleViolation>> FLintRuleViolation::AllRuleViolationsWithViolatorShared(const TArray<FLintRuleViolation>& RuleViolationCollection, const UObject* SearchViolator)
{
	// This should really be done when the structs are first created
	TArray<TSharedPtr<FLintRuleViolation>> SharedViolations;
	TArray<FLintRuleViolation> Violations = AllRuleViolationsWithViolator(RuleViolationCollection, SearchViolator);
	for (FLintRuleViolation Violation : Violations)
	{
		SharedViolations.Push(TSharedPtr<FLintRuleViolation>(new FLintRuleViolation(Violation)));
	}

	return SharedViolations;
}

TArray<FLintRuleViolation> FLintRuleViolation::AllRuleViolationsOfSpecificRule(const TArray<FLintRuleViolation>& RuleViolationCollection, TSubclassOf<ULintRule> SearchRule)
{
	return RuleViolationCollection.FilterByPredicate([SearchRule](const FLintRuleViolation& RuleViolation)
		{
			if (SearchRule.Get() != nullptr && RuleViolation.ViolatedRule == SearchRule)
			{
				return true;
			}

			return false;
		});
}

TArray<FLintRuleViolation> FLintRuleViolation::AllRuleViolationsOfRuleGroup(const TArray<FLintRuleViolation>& RuleViolationCollection, FName SearchRuleGroup)
{
	return RuleViolationCollection.FilterByPredicate([SearchRuleGroup](const FLintRuleViolation& RuleViolation)
		{
			if (RuleViolation.ViolatedRule.Get() != nullptr && RuleViolation.ViolatedRule.GetDefaultObject()->RuleGroup == SearchRuleGroup)
			{
				return true;
			}

			return false;
		});
}

TArray<UObject*> FLintRuleViolation::AllRuleViolationViolators(const TArray<FLintRuleViolation>& RuleViolationCollection)
{
	TArray<UObject*> Violators;
	for (const FLintRuleViolation& RuleViolation : RuleViolationCollection)
	{
		Violators.Add(RuleViolation.Violator.Get());
	}
	return Violators;
}

TArray<UObject*> FLintRuleViolation::AllRuleViolationViolators(const TArray<TSharedPtr<FLintRuleViolation>>& RuleViolationCollection)
{
	TArray<UObject*> Violators;
	for (const TSharedPtr<FLintRuleViolation>& RuleViolation : RuleViolationCollection)
	{
		Violators.AddUnique(RuleViolation->Violator.Get());
	}
	return Violators;
}

TMultiMap<UObject*, FLintRuleViolation> FLintRuleViolation::AllRuleViolationsMappedByViolator(const TArray<FLintRuleViolation>& RuleViolationCollection)
{
	TMultiMap<UObject*, FLintRuleViolation> ViolatorViolationsMultiMap;

	for (const FLintRuleViolation& RuleViolation : RuleViolationCollection)
	{
		ViolatorViolationsMultiMap.Add(RuleViolation.Violator.Get(), RuleViolation);
	}

	return ViolatorViolationsMultiMap;
}

TMultiMap<ULintRule*, FLintRuleViolation> FLintRuleViolation::AllRuleViolationsMappedByViolatedLintRule(const TArray<FLintRuleViolation>& RuleViolationCollection)
{
	TMultiMap<ULintRule*, FLintRuleViolation> LintRuleViolationsMultiMap;

	for (const FLintRuleViolation& RuleViolation : RuleViolationCollection)
	{
		LintRuleViolationsMultiMap.Add(RuleViolation.ViolatedRule.GetDefaultObject(), RuleViolation);
	}

	return LintRuleViolationsMultiMap;
}

TMultiMap<const ULintRule*, TSharedPtr<FLintRuleViolation>> FLintRuleViolation::AllRuleViolationsMappedByViolatedLintRuleShared(const TArray<FLintRuleViolation>& RuleViolationCollection)
{
	// We should really just create shared ptrs to begin with instead of doing this
	TMultiMap<const ULintRule*, TSharedPtr<FLintRuleViolation>> LintRuleViolationsMultiMap;

	for (const FLintRuleViolation& RuleViolation : RuleViolationCollection)
	{
		LintRuleViolationsMultiMap.Add(RuleViolation.ViolatedRule.GetDefaultObject(), TSharedPtr<FLintRuleViolation>(new FLintRuleViolation(RuleViolation)));
	}

	return LintRuleViolationsMultiMap;
}

TMultiMap<const ULintRule*, TSharedPtr<FLintRuleViolation>> FLintRuleViolation::AllRuleViolationsMappedByViolatedLintRuleShared(const TArray<TSharedPtr<FLintRuleViolation>>& RuleViolationCollection)
{
	TMultiMap<const ULintRule*, TSharedPtr<FLintRuleViolation>> LintRuleViolationsMultiMap;

	for (const TSharedPtr<FLintRuleViolation>& RuleViolation : RuleViolationCollection)
	{
		LintRuleViolationsMultiMap.Add(RuleViolation->ViolatedRule.GetDefaultObject(), RuleViolation);
	}

	return LintRuleViolationsMultiMap;
}

bool FLintRuleViolation::PopulateAssetData()
{
	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	TArray<FAssetRenameData> AssetRenameData;

	if (Violator.IsValid())
	{
		ViolatorAssetData = AssetRegistry.GetAssetByObjectPath(FName(*Violator->GetPathName()));
		return ViolatorAssetData.IsValid();
	}

	return false;
}
