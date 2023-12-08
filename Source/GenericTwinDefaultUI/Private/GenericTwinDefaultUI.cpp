// Copyright Epic Games, Inc. All Rights Reserved.

#include "GenericTwinDefaultUI.h"
#include "Layers/LayerRepository.h"

#include "DefaultUILayer.h"

#define LOCTEXT_NAMESPACE "FGenericTwinDefaultUIModule"

void FGenericTwinDefaultUIModule::StartupModule()
{
	LayerRepository::GetInstance().RegisterLayer(UDefaultUILayer::GetLayerMetaData(), &UDefaultUILayer::Create);
}

void FGenericTwinDefaultUIModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FGenericTwinDefaultUIModule::PostLoadCallback()
{
	LayerRepository::GetInstance().RegisterLayer(UDefaultUILayer::GetLayerMetaData(), &UDefaultUILayer::Create);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGenericTwinDefaultUIModule, GenericTwinDefaultUI)