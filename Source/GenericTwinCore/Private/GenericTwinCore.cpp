// Copyright Epic Games, Inc. All Rights Reserved.

#include "GenericTwinCore.h"

#include "Common/Communication/GenericTwinHttpRequestManager.h"

#define LOCTEXT_NAMESPACE "FGenericTwinCoreModule"

void FGenericTwinCoreModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .u file per-module
}

void FGenericTwinCoreModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FGenericTwinHttpRequestManager::Shutdown();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGenericTwinCoreModule, GenericTwinCore)