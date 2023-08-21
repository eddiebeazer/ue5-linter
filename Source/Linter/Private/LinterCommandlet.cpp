// Copyright 2020 Gamemakin LLC. All Rights Reserved.

#include "LinterCommandlet.h"
#include "Editor.h"
#include "AssetRegistryModule.h"
#include "AssetData.h"
#include "Engine/ObjectLibrary.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Linter.h"
#include "LinterSettings.h"
#include "LintRule.h"
#include "LintRuleSet.h"

DEFINE_LOG_CATEGORY_STATIC(LinterCommandlet, All, All);

ULinterCommandlet::ULinterCommandlet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	IsClient = false;
	IsServer = false;
}

static void PrintUsage()
{
	UE_LOG(LinterCommandlet, Display, TEXT("Linter Usage: {Editor}.exe Project.uproject -run=Linter \"/Game/\""));
	UE_LOG(LinterCommandlet, Display, TEXT(""));
	UE_LOG(LinterCommandlet, Display, TEXT("This will run the Linter on the provided project and will scan the supplied directory, example being the project's full Content/Game tree. Can add multiple paths as additional arguments."));
}

int32 ULinterCommandlet::Main(const FString& InParams)
{
	FString Params = InParams;
	// Parse command line.
	TArray<FString> Paths;
	TArray<FString> Switches;
	TMap<FString, FString> ParamsMap;
	ParseCommandLine(*Params, Paths, Switches, ParamsMap);

	UE_LOG(LinterCommandlet, Display, TEXT("Linter is indeed running!"));
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	UE_LOG(LinterCommandlet, Display, TEXT("Loading the asset registry..."));
	AssetRegistryModule.Get().SearchAllAssets(/*bSynchronousSearch =*/true);
	UE_LOG(LinterCommandlet, Display, TEXT("Finished loading the asset registry. Determining Rule Set..."));

	ULintRuleSet* RuleSet = GetDefault<ULinterSettings>()->DefaultLintRuleSet.LoadSynchronous();
	if (ParamsMap.Contains(TEXT("RuleSet")))
	{
		const FString RuleSetName = *ParamsMap.FindChecked(TEXT("RuleSet"));
		UE_LOG(LinterCommandlet, Display, TEXT("Trying to find Rule Set with Commandlet Name: %s"), *RuleSetName);

		FLinterModule::TryToLoadAllLintRuleSets();

		TArray<FAssetData> FoundRuleSets;
		AssetRegistryModule.Get().GetAssetsByClass(ULintRuleSet::StaticClass()->GetFName(), FoundRuleSets, true);

		for (const FAssetData& RuleSetData : FoundRuleSets)
		{
			ULintRuleSet* LoadedRuleSet = Cast<ULintRuleSet>(RuleSetData.GetAsset());
			if (LoadedRuleSet != nullptr && LoadedRuleSet->NameForCommandlet == RuleSetName)
			{
				RuleSet = LoadedRuleSet;
				UE_LOG(LinterCommandlet, Display, TEXT("Found Rule Set for name %s: %s"), *RuleSetName, *RuleSet->GetFullName());
			}
		}
	}
	else
	{
		UE_LOG(LinterCommandlet, Display, TEXT("Using default rule set..."));
	}

	if (RuleSet == nullptr)
	{
		UE_LOG(LinterCommandlet, Error, TEXT("Failed to load a rule set. Aborting. Returning error code 1."));
		return 1;
	}

	UE_LOG(LinterCommandlet, Display, TEXT("Using rule set: %s"), *RuleSet->GetFullName());

	if (Paths.Num() == 0)
	{
		Paths.Add(TEXT("/Game"));
	}

	UE_LOG(LinterCommandlet, Display, TEXT("Attempting to Lint paths: %s"), *FString::Join(Paths, TEXT(", ")));

	const TArray<FLintRuleViolation> RuleViolations = RuleSet->LintPath(Paths);

	int32 NumErrors = 0;
	int32 NumWarnings = 0;

	for (const FLintRuleViolation& Violation : RuleViolations)
	{
		if (Violation.ViolatedRule->GetDefaultObject<ULintRule>()->RuleSeverity <= ELintRuleSeverity::Error)
		{
			NumErrors++;
		}
		else
		{
			NumWarnings++;
		}
	}

	FString ResultsString = FText::FormatNamed(FText::FromString("Lint completed with {NumWarnings} {NumWarnings}|plural(one=warning,other=warnings), {NumErrors} {NumErrors}|plural(one=error,other=errors)."), TEXT("NumWarnings"), FText::FromString(FString::FromInt(NumWarnings)), TEXT("NumErrors"), FText::FromString(FString::FromInt(NumErrors))).ToString();
	UE_LOG(LinterCommandlet, Display, TEXT("Lint completed with %s."), *ResultsString);

	bool bWriteReport = Switches.Contains(TEXT("json")) || ParamsMap.Contains(TEXT("json")) || Switches.Contains(TEXT("html")) || ParamsMap.Contains(TEXT("html"));
	if (bWriteReport)
	{
		UE_LOG(LinterCommandlet, Display, TEXT("Generating output report..."));

		TSharedPtr<FJsonObject> RootJsonObject = MakeShareable(new FJsonObject);
		TArray<TSharedPtr<FJsonValue>> ViolatorJsonObjects;

		TArray<UObject*> UniqueViolators = FLintRuleViolation::AllRuleViolationViolators(RuleViolations);
		for (const UObject* Violator : UniqueViolators)
		{
			TSharedPtr<FJsonObject> AssetJsonObject = MakeShareable(new FJsonObject);
			TArray<FLintRuleViolation> UniqueViolatorViolations = FLintRuleViolation::AllRuleViolationsWithViolator(RuleViolations, Violator);

			FAssetData AssetData;
			if (UniqueViolatorViolations.Num() > 0)
			{
				UniqueViolatorViolations[0].PopulateAssetData();
				AssetData = UniqueViolatorViolations[0].ViolatorAssetData;
				AssetJsonObject->SetStringField(TEXT("ViolatorAssetName"), AssetData.AssetName.ToString());
				AssetJsonObject->SetStringField(TEXT("ViolatorAssetPath"), AssetData.ObjectPath.ToString());
				AssetJsonObject->SetStringField(TEXT("ViolatorFullName"), AssetData.GetFullName());
				//@TODO: Thumbnail export?

				TArray<TSharedPtr<FJsonValue>> RuleViolationJsonObjects;

				for (const FLintRuleViolation& Violation : UniqueViolatorViolations)
				{
					ULintRule* LintRule = Violation.ViolatedRule->GetDefaultObject<ULintRule>();
					check(LintRule != nullptr);

					TSharedPtr<FJsonObject> RuleJsonObject = MakeShareable(new FJsonObject);
					RuleJsonObject->SetStringField(TEXT("RuleGroup"), LintRule->RuleGroup.ToString());
					RuleJsonObject->SetStringField(TEXT("RuleTitle"), LintRule->RuleTitle.ToString());
					RuleJsonObject->SetStringField(TEXT("RuleDesc"), LintRule->RuleDescription.ToString());
					RuleJsonObject->SetStringField(TEXT("RuleURL"), LintRule->RuleURL);
					RuleJsonObject->SetNumberField(TEXT("RuleSeverity"), (int32)LintRule->RuleSeverity);
					RuleJsonObject->SetStringField(TEXT("RuleRecommendedAction"), Violation.RecommendedAction.ToString());
					RuleViolationJsonObjects.Push(MakeShareable(new FJsonValueObject(RuleJsonObject)));
				}

				AssetJsonObject->SetArrayField(TEXT("Violations"), RuleViolationJsonObjects);
			}

			ViolatorJsonObjects.Add(MakeShareable(new FJsonValueObject(AssetJsonObject)));

		}

		// Save off our JSON to a string
		RootJsonObject->SetArrayField(TEXT("Violators"), ViolatorJsonObjects);
		FString JsonReport;
		TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonReport);
		FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), Writer);

		// write json file if requested
		if (Switches.Contains(TEXT("json")) || ParamsMap.Contains(FString(TEXT("json"))))
		{
			FDateTime Now = FDateTime::Now();
			FString JsonOutputName = TEXT("lint-report-") + FDateTime::Now().ToString() + TEXT(".json");

			const FString LintReportPath = FPaths::ProjectSavedDir() / TEXT("LintReports");
			FString FullOutputPath = LintReportPath / JsonOutputName;

			if (ParamsMap.Contains(FString(TEXT("json"))))
			{
				const FString JsonOutputOverride = *ParamsMap.FindChecked(FString(TEXT("json")));
				if (FPaths::IsRelative(JsonOutputOverride))
				{
					JsonOutputName = JsonOutputOverride;
					FullOutputPath = LintReportPath / JsonOutputName;
				}
				else
				{
					FullOutputPath = JsonOutputOverride;
				}
			}

			FullOutputPath = FPaths::ConvertRelativePathToFull(FullOutputPath);
			IFileManager::Get().MakeDirectory(*FPaths::GetPath(FullOutputPath), true);

			UE_LOG(LinterCommandlet, Display, TEXT("Exporting JSON report to %s"), *FullOutputPath);
			if (FFileHelper::SaveStringToFile(JsonReport, *FullOutputPath))
			{
				UE_LOG(LinterCommandlet, Display, TEXT("Exported JSON report successfully."));
			}
			else
			{
				UE_LOG(LinterCommandlet, Error, TEXT("Failed to export JSON report. Aborting. Returning error code 1."));
				return 1;
			}
		}

		// write HTML report if requested
		if (Switches.Contains(TEXT("html")) || ParamsMap.Contains(FString(TEXT("html"))))
		{
			FDateTime Now = FDateTime::Now();
			FString HtmlOutputName = TEXT("lint-report-") + FDateTime::Now().ToString() + TEXT(".html");

			const FString LintReportPath = FPaths::ProjectSavedDir() / TEXT("LintReports");
			FString FullOutputPath = LintReportPath / HtmlOutputName;

			if (ParamsMap.Contains(FString(TEXT("html"))))
			{
				const FString HtmlOutputOverride = *ParamsMap.FindChecked(TEXT("html"));
				if (FPaths::IsRelative(HtmlOutputName))
				{
					HtmlOutputName = HtmlOutputOverride;
					FullOutputPath = LintReportPath / HtmlOutputName;
				}
				else
				{
					FullOutputPath = HtmlOutputOverride;
				}
			}

			FullOutputPath = FPaths::ConvertRelativePathToFull(FullOutputPath);
			IFileManager::Get().MakeDirectory(*FPaths::GetPath(FullOutputPath), true);
			UE_LOG(LinterCommandlet, Display, TEXT("Exporting HTML report to %s"), *FullOutputPath);

			FString TemplatePath = FPaths::Combine(*IPluginManager::Get().FindPlugin(TEXT("Linter"))->GetBaseDir(), TEXT("Resources"), TEXT("LintReportTemplate.html"));
			UE_LOG(LinterCommandlet, Display, TEXT("Loading HTML report template from %s"), *TemplatePath);

			FString HTMLReport;
			if (FFileHelper::LoadFileToString(HTMLReport, *TemplatePath))
			{
				UE_LOG(LinterCommandlet, Display, TEXT("Loading HTML report template successfully."));

				HTMLReport.ReplaceInline(TEXT("{% TITLE %}"), *FPaths::GetBaseFilename(FPaths::GetProjectFilePath()));
				HTMLReport.ReplaceInline(TEXT("{% RESULTS %}"), *ResultsString);
				HTMLReport.ReplaceInline(TEXT("{% LINT_REPORT %}"), *JsonReport);
			}
			else
			{
				UE_LOG(LinterCommandlet, Error, TEXT("Failed to load HTML report template."));
				return 1;
			}

			if (FFileHelper::SaveStringToFile(HTMLReport, *FullOutputPath))
			{
				UE_LOG(LinterCommandlet, Display, TEXT("Exported HTML report successfully."));
			}
			else
			{
				UE_LOG(LinterCommandlet, Error, TEXT("Failed to export HTML report. Aborting. Returning error code 1."));
				return 1;
			}
		}
	}
	if (Switches.Contains(TEXT("IgnoreErrors")))
	{
		return 0;
	}

	if (NumErrors > 0 || Switches.Contains(TEXT("TreatWarningsAsErrors")) && NumWarnings > 0)
	{
		UE_LOG(LinterCommandlet, Display, TEXT("Lint completed with errors. Returning error code 1."));
		return 1;
	}

	return 0;
}