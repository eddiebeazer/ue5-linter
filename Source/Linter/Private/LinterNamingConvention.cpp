#include "LinterNamingConvention.h"
#include "DetailLayoutBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Templates/SharedPointer.h"
#include "DetailCategoryBuilder.h"
#include "IDetailChildrenBuilder.h"

TSharedRef<IDetailCustomization> FLinterNamingConventionDetails::MakeInstance()
{
	return MakeShareable(new FLinterNamingConventionDetails());
}

void FLinterNamingConventionDetails::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder)
{
	// Edit the Conventions category
	IDetailCategoryBuilder& DetailCategory = DetailBuilder.EditCategory("Conventions");
	TSharedRef<IPropertyHandle> NamingConventionsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(ULinterNamingConvention, ClassNamingConventions), ULinterNamingConvention::StaticClass());

	TSharedRef<FDetailArrayBuilder> ConventionsPropertyBuilder = MakeShareable(new FDetailArrayBuilder(NamingConventionsProperty));
	ConventionsPropertyBuilder->OnGenerateArrayElementWidget(FOnGenerateArrayElementWidget::CreateSP(this, &FLinterNamingConventionDetails::OnGenerateElementForDetails, &DetailBuilder));


	DetailCategory.AddCustomBuilder(ConventionsPropertyBuilder);
}

void FLinterNamingConventionDetails::OnGenerateElementForDetails(TSharedRef<IPropertyHandle> StructProperty, int32 ElementIndex, IDetailChildrenBuilder& ChildrenBuilder, IDetailLayoutBuilder* DetailLayout)
{
	ChildrenBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(SProperty, StructProperty->GetChildHandle("SoftClassPtr"))
			.ShouldDisplayName(false)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		[
			SNew(SProperty, StructProperty->GetChildHandle("Variant"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		[
			SNew(SProperty, StructProperty->GetChildHandle("Prefix"))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		[
			SNew(SProperty, StructProperty->GetChildHandle("Suffix"))
		]
	];
}

ULinterNamingConvention::ULinterNamingConvention(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ClassNamingConventions = TArray<FLinterNamingConventionInfo>();
}

TArray<FLinterNamingConventionInfo> ULinterNamingConvention::GetNamingConventionsForClassVariant(TSoftClassPtr<UObject> Class, FName Variant /*= NAME_None*/) const
{
	TArray<FLinterNamingConventionInfo> NamingConventionList;

	UClass* searchClass = Class.Get();
	while (NamingConventionList.Num() == 0 && searchClass != nullptr)
	{
		NamingConventionList = ClassNamingConventions.FilterByPredicate([searchClass, Variant](const FLinterNamingConventionInfo& Info)
			{
				return (Info.SoftClassPtr.Get() == searchClass && Info.Variant == Variant);
			});

		// Abort if we try to go above UObject
		if (searchClass == UObject::StaticClass())
		{
			break;
		}

		// @HACK: Editor UI won't allow us to select the UObject class in some cases
		if (searchClass == UAnyObject_LinterDummyClass::StaticClass())
		{
			searchClass = UObject::StaticClass();
			continue;
		}

		// Load our parent class in case we failed to get naming conventions
		searchClass = searchClass->GetSuperClass();
	}

	return NamingConventionList;	
}

void ULinterNamingConvention::SortConventions()
{
	ClassNamingConventions.Sort([](const FLinterNamingConventionInfo& A, const FLinterNamingConventionInfo& B)
	{
		if (A.SoftClassPtr.GetAssetName() < B.SoftClassPtr.GetAssetName())
		{
			return true;
		}

		if (A.SoftClassPtr.GetAssetName() == B.SoftClassPtr.GetAssetName())
		{
			int32 sort = A.Variant.Compare(B.Variant);
			if (sort < 0)
			{
				return true;
			}

			if (sort == 0)
			{
				sort = A.Prefix.Compare(B.Prefix);
				if (sort < 0)
				{
					return true;
				}

				if (sort == 0)
				{
					sort = A.Suffix.Compare(B.Suffix);
					if (sort <= 0)
					{
						return true;
					}
					return false;
				}

				return false;
			}

			return false;
		}

		return false;
	});
}
