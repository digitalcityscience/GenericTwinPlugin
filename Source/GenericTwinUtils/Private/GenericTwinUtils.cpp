// Copyright Epic Games, Inc. All Rights Reserved.

#include "GenericTwinUtils.h"

#define LOCTEXT_NAMESPACE "FGenericTwinUtilsModule"

void FGenericTwinUtilsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .u file per-module
}

void FGenericTwinUtilsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGenericTwinUtilsModule, GenericTwinUtils)