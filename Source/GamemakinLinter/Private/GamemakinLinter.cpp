// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#include "GamemakinLinter.h"

#define LOCTEXT_NAMESPACE "FGamemakinLinterModule"

void FGamemakinLinterModule::StartupModule()
{
//	Super::StartupModule();
}

void FGamemakinLinterModule::ShutdownModule()
{
//	Super::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGamemakinLinterModule, GamemakinLinter)
DEFINE_LOG_CATEGORY(LogGamemakinLinter);