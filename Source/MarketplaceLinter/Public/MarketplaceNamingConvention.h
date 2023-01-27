// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Templates/SharedPointer.h"
#include "LinterNamingConvention.h"
#include "MarketplaceNamingConvention.generated.h"


UCLASS()
class UMarketplaceNamingConvention : public ULinterNamingConvention
{
	GENERATED_BODY()

public:

	UMarketplaceNamingConvention(const FObjectInitializer& ObjectInitializer);

};
