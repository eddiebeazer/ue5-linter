// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/SharedPointer.h"
#include "IDetailCustomization.h"
#include "PropertyHandle.h"
#include "LinterNamingConvention.generated.h"


/**
 * Class/Prefix/Suffix settings for Linter
 */
USTRUCT(BlueprintType)
struct LINTER_API FLinterNamingConventionInfo
{
	GENERATED_USTRUCT_BODY()

	FLinterNamingConventionInfo()
		: SoftClassPtr(nullptr)
	{}

	FLinterNamingConventionInfo(TSoftClassPtr<UObject> InClass, FString InPrefix = TEXT(""), FString InSuffix = TEXT(""), FName InVariant = NAME_None)
		: SoftClassPtr(InClass)
		, Prefix(InPrefix)
		, Suffix(InSuffix)
		, Variant(InVariant)
	{}

	UPROPERTY(EditAnywhere, Category = Default, meta = (AllowAbstract = ""))
	TSoftClassPtr<UObject> SoftClassPtr;

	UPROPERTY(EditAnywhere, Category = Default)
	FString Prefix;

	UPROPERTY(EditAnywhere, Category = Default)
	FString Suffix;
	
	UPROPERTY(EditAnywhere, Category = Default)
	FName Variant;
};

class FLinterNamingConventionDetails : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** ILayoutDetails interface */
	virtual void CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder) override;

	void OnGenerateElementForDetails(TSharedRef<IPropertyHandle> StructProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder, IDetailLayoutBuilder* DetailLayout);
};

/**
* Contains a naming convention to be used by LinterManagers/LinterRules
*/
UCLASS(Abstract)
class LINTER_API ULinterNamingConvention : public UDataAsset
{
	GENERATED_BODY()

public:

	ULinterNamingConvention(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category="Conventions", meta = (AllowAbstract = ""))
	TArray<FLinterNamingConventionInfo> ClassNamingConventions;

	UFUNCTION(BlueprintCallable, Category="Conventions")
	TArray<FLinterNamingConventionInfo> GetNamingConventionsForClassVariant(TSoftClassPtr<UObject> Class, FName Variant = NAME_None) const;

	UFUNCTION(Blueprintcallable, Category = "Conventions")
	void SortConventions();

protected:

	

};
