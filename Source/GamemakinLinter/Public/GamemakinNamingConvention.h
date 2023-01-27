// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "LinterNamingConvention.h"
#include "GamemakinNamingConvention.generated.h"

UCLASS()
class UGamemakinNamingConvention : public ULinterNamingConvention
{
	GENERATED_BODY()

public:

	UGamemakinNamingConvention(const FObjectInitializer& ObjectInitializer);

};
