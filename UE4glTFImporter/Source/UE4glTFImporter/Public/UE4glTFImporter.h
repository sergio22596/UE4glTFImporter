// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(ImporterLog, Log, All);

#define SHOW_INFO_LOGGER 1
#define SHOW_WARNING_LOGGER 1
#define SHOW_ERROR_LOGGER 1

class FUE4glTFImporterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};