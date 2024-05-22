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

#include "Layers/LayerRepository.h"

DEFINE_LOG_CATEGORY(LogLayerRepository);

LayerRepository& LayerRepository::GetInstance()
{
	static LayerRepository theInstance;

	return theInstance;
}

void LayerRepository::RegisterLayer(const SLayerMetaData &LayerMetaData, CreateLayerFuncPtr CreateLayerFunc)
{
	if(m_RegisteredLayers.find(LayerMetaData.LayerType) == m_RegisteredLayers.end())
	{
		m_RegisteredLayers[LayerMetaData.LayerType] = SLayerInfo(LayerMetaData, CreateLayerFunc);
		UE_LOG(LogLayerRepository, Log, TEXT("Layer %s successfully registered"), *(LayerMetaData.LayerType));
	}
	else
	{
		m_RegisteredLayers[LayerMetaData.LayerType] = SLayerInfo(LayerMetaData, CreateLayerFunc);
		UE_LOG(LogLayerRepository, Warning, TEXT("Layer %s already registered"), *(LayerMetaData.LayerType));
	}
}

void LayerRepository::RegisterLayer(const SLayerMetaData &LayerMetaData, CreateLayerFuncPtr CreateLayerFunc, GetInitialConfigFuncPtr GetInitialConfigFunc)
{
	if(m_RegisteredLayers.find(LayerMetaData.LayerType) == m_RegisteredLayers.end())
	{
		m_RegisteredLayers[LayerMetaData.LayerType] = SLayerInfo(LayerMetaData, CreateLayerFunc, GetInitialConfigFunc);
		UE_LOG(LogLayerRepository, Log, TEXT("Layer %s successfully registered"), *(LayerMetaData.LayerType));
	}
	else
	{
		m_RegisteredLayers[LayerMetaData.LayerType] = SLayerInfo(LayerMetaData, CreateLayerFunc, GetInitialConfigFunc);
		UE_LOG(LogLayerRepository, Warning, TEXT("Layer %s already registered"), *(LayerMetaData.LayerType));
	}
}

TArray<SLayerMetaData> LayerRepository::GetAvailableLayers() const
{
	TArray<SLayerMetaData> layers;
	for(const auto &item : m_RegisteredLayers)
	{
		layers.Add(item.second.LayerMetaData);
	}
	return layers;
}

TSharedPtr<FJsonObject> LayerRepository::GetInitialConfiguration(const FString &LayerType) const
{
	std::map<FString, SLayerInfo>::const_iterator it = m_RegisteredLayers.find(LayerType);
	if (it != m_RegisteredLayers.end())
	{
		const SLayerInfo &li = it->second;
		if(li.GetInitialConfigFunc)
			return li.GetInitialConfigFunc();
	}

	return TSharedPtr<FJsonObject>();
}

ULayerBase* LayerRepository::CreateLayer(FTwinWorldContext &TwinWorldCtx, const FString &LayerType, const TSharedPtr<FJsonObject> &Parameters)
{
	std::map<FString, SLayerInfo>::iterator it = m_RegisteredLayers.find(LayerType);
	if(it != m_RegisteredLayers.end())
	{
		return it->second.CreateLayerFunc(TwinWorldCtx, Parameters);
	}

	UE_LOG(LogLayerRepository, Warning, TEXT("Layer %s not registered"), *LayerType);
	return 0;
}
