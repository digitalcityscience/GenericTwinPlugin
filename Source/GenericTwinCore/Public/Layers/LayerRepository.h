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

#pragma once

#include "CoreMinimal.h"
#include "Layers/LayerTypes.h"

#include <map>
#include <functional>

DECLARE_LOG_CATEGORY_EXTERN(LogLayerRepository, Log, All);

class ULayerBase;

class FJsonObject;

struct FTwinWorldContext;

class GENERICTWINCORE_API LayerRepository
{
public:

	typedef std::function< ULayerBase*(FTwinWorldContext&, const FJsonObject&) > CreateLayerFuncPtr;

	typedef std::function< TSharedPtr<FJsonObject>() > GetInitialConfigFuncPtr;

private:

	struct SLayerInfo
	{
		SLayerInfo()
		{	}

		SLayerInfo(const SLayerMetaData &_LayerMetaData, CreateLayerFuncPtr _CreateLayerFunc)
			:	LayerMetaData(_LayerMetaData)
			,	CreateLayerFunc(_CreateLayerFunc)
			,	GetInitialConfigFunc()
		{	}

		SLayerInfo(const SLayerMetaData &_LayerMetaData, CreateLayerFuncPtr _CreateLayerFunc, GetInitialConfigFuncPtr _GetInitialConfigFunc)
			:	LayerMetaData(_LayerMetaData)
			,	CreateLayerFunc(_CreateLayerFunc)
			,	GetInitialConfigFunc(_GetInitialConfigFunc)
		{	}

		SLayerMetaData				LayerMetaData;

		CreateLayerFuncPtr			CreateLayerFunc;

		GetInitialConfigFuncPtr		GetInitialConfigFunc;
	};

public:

	static LayerRepository& GetInstance();

	void RegisterLayer(const SLayerMetaData &LayerMetaData, CreateLayerFuncPtr CreateLayerFunc);

	void RegisterLayer(const SLayerMetaData &LayerMetaData, CreateLayerFuncPtr CreateLayerFunc, GetInitialConfigFuncPtr GetInitialConfigFunc);

	TArray<SLayerMetaData>  GetAvailableLayers() const;

	TSharedPtr<FJsonObject> GetInitialConfiguration(const FString &LayerName) const;

	ULayerBase* CreateLayer(FTwinWorldContext &TwinWorldCtx, const FString &LayerType, const FJsonObject &Parameters);

private:

	std::map<FString, SLayerInfo>		m_RegisteredLayers;

};
