// Copyright 2019-2020 Gamemakin LLC. All Rights Reserved.

#pragma once
#include "Linter.h"


// Integrate Linter actions into the Content Browser
class FLinterContentBrowserExtensions
{
public:
	static void InstallHooks(FLinterModule* LinterModule, FDelegateHandle* pContentBrowserExtenderDelegateHandle, FDelegateHandle*
	                         pAssetExtenderDelegateHandle);
	static void RemoveHooks(FLinterModule* LinterModule, class FDelegateHandle* pContentBrowserExtenderDelegateHandle, class FDelegateHandle* pAssetExtenderDelegateHandle);
};