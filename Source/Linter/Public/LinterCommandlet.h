// Copyright 2016 Gamemakin LLC. All Rights Reserved.

#pragma once
#include "Commandlets/Commandlet.h"
#include "LinterCommandlet.generated.h"

UCLASS()
class ULinterCommandlet : public UCommandlet
{
	GENERATED_UCLASS_BODY()
	//~ Begin UCommandlet Interface
	virtual int32 Main(const FString& Params) override;

	//~ End UCommandlet Interface
};


