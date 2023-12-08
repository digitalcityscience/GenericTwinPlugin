/**

The MIT License (MIT)

Copyright (c) 2023 - present, Hamburg Port Authority


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

**/

#include "Layers/LayerManager.h"
#include "Layers/LayerRepository.h"

#include "Layers/Dependencies/LayerDependencies.h"

#include "Configuration/MainConfiguration.h"
#include "Configuration/LayersConfiguration.h"

#include "Common/Material/GenericTwinMaterialManager.h"
#include "Common/Material/GenericTwinTextureCache.h"
#include "Common/Caching/LocalFileCache.h"

#include "Common/JobQueue/GenericTwinJobDispatcher.h"

#include "Kismet/GameplayStatics.h"
#include "Dom/JsonObject.h"

#include "GeoReferencingSystem.h"

DEFINE_LOG_CATEGORY(LogLayerManager);

ULayerManager::ULayerManager()
{
}

void ULayerManager::Initialize(FTwinWorldContext &TwinWorldContext, const struct SLayersConfiguration &LayersConfiguration)
{
	m_TwinWorldContext = &TwinWorldContext;

	LayerRepository &layerRep = LayerRepository::GetInstance();

	for(const SLayerConfiguration &layerCfg : LayersConfiguration.Layers)
	{
		ULayerBase *layer = layerRep.CreateLayer(*m_TwinWorldContext, layerCfg.Type, layerCfg.Parameters.IsValid() ? (*layerCfg.Parameters) : FJsonObject());
		if(layer)
		{
			layer->SetCustomName(layerCfg.CustomName);
			
			if(layerCfg.Dependency)
			{
				SetupLayerDependency(*layer, (*layerCfg.Dependency));
			}
			
			UE_LOG(LogLayerManager, Log, TEXT("Layer of type %s created with custom name of '%s'"), *(layerCfg.Type), *(layerCfg.CustomName));
			m_LayersToInitialize.Add(layer);
		}
	}
}

void ULayerManager::Update(float DeltaSeconds)
{
	if(m_TwinWorldContext)
	{
		if(m_LayersToInitialize.Num() > 0)
		{
			ULayerBase *layer = m_LayersToInitialize[0];
			if(layer)
			{
				AddLayer(*layer);
			}

			m_LayersToInitialize.RemoveAt(0);
		}

		for(ULayerBase *layer : m_ActiveLayers)
		{
			layer->UpdateView(*m_TwinWorldContext, DeltaSeconds);
		}
	}
}

void ULayerManager::Shutdown()
{
	for(ULayerBase *layer : m_ActiveLayers)
	{
		layer->OnEndPlay(*m_TwinWorldContext);
	}
}

void ULayerManager::CreateLayer(const FString &LayerType, const FJsonObject &Parameters)
{
	if(m_TwinWorldContext)
	{
		ULayerBase *layer = LayerRepository::GetInstance().CreateLayer(*m_TwinWorldContext, LayerType, Parameters);
		if(layer)
		{
			UE_LOG(LogLayerManager, Log, TEXT("Layer %s created"), *(LayerType));
			m_LayersToInitialize.Add(layer);
		}
	}
}

void ULayerManager::AddLayer(ULayerBase &Layer)
{
	Layer.InitializeLayer(*m_TwinWorldContext);
	m_Layers.Add(m_nextLayerId, &Layer);
	Layer.SetLayerId(m_nextLayerId);
	++m_nextLayerId;

	m_ActiveLayers.Add(&Layer);
	m_ActiveLayers.Sort([](const ULayerBase& a, const ULayerBase& b) { return a.GetLayerPriority() < b.GetLayerPriority(); });

	OnLayerListUpdated.Broadcast();
}

void ULayerManager::SetupLayerDependency(ULayerBase &Layer, const FJsonObject &DependencyObject)
{
	if	(	DependencyObject.HasTypedField<EJson::String>(TEXT("type"))
		&&	DependencyObject.HasTypedField<EJson::Array>(TEXT("depends_on"))
		)
	{
		const FString type = DependencyObject.GetStringField(TEXT("type"));

		TArray<FString> dependsOn;
		for(const TSharedPtr<FJsonValue> &valPtr  : DependencyObject.GetArrayField(TEXT("depends_on")))
		{
			if(valPtr->Type == EJson::String)
			{
				dependsOn.Add(valPtr->AsString());
			}
		}

		if(dependsOn.Num() > 0)
		{
			GenericTwin::LayerDependencyBasePtr dependencyPtr = GenericTwin::CreateDependency(type);

			if(dependencyPtr)
			{
				if(dependencyPtr->Configure(DependencyObject))
				{
					UE_LOG(LogLayerManager, Log, TEXT("Adding dependency of type %s to layer"), *(type));

					dependencyPtr->SetDependsOn(dependsOn);
					dependencyPtr->SetLayerManager(*this);

					Layer.AddLayerDependency(dependencyPtr);
				}
				else
				{
					UE_LOG(LogLayerManager, Log, TEXT("Failed to configure dependency of type %s"), *(type));
				}
			}
			else
			{
				UE_LOG(LogLayerManager, Log, TEXT("Couldn't create dependency of type %s"), *(type));
			}
		}
	}
}

void ULayerManager::SetLayerVisibility(int32 LayerId, bool NewVisibility)
{
	if	(	m_TwinWorldContext
		&&	m_Layers.Contains(LayerId)
		)
	{
		m_Layers[LayerId]->SetVisibility(*m_TwinWorldContext, NewVisibility);
	}
}

bool ULayerManager::HasLayerOfType(const FString LayerType) const
{
	for(const auto &it : m_Layers)
	{
		if(it.Value->GetLayerType() == LayerType)
			return true;
	}
	return false;
}

