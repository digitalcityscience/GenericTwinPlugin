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
#include "UObject/Object.h"
#include "LayerManager.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogLayerManager, Log, All);

class ULayerBase;

struct FTwinWorldContext;

/**
 * 
 */
UCLASS()
class GENERICTWINCORE_API ULayerManager : public UObject
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLayerListUpdated);

	ULayerManager();

public:	

	void Initialize(FTwinWorldContext &TwinWorldContext, const struct SLayersConfiguration &LayersConfiguration);

	void Update(float DeltaSeconds);

	void Shutdown();

	const TMap<int32, ULayerBase*>& GetLayers() const;

	void SetLayerVisibility(int32 LayerId, bool NewVisibility);

	bool HasLayerOfType(const FString LayerType) const;
	ULayerBase* GetLayer(int32 LayerId) const;

	void CreateLayer(const FString &LayerType, const TSharedPtr<FJsonObject> &Parameters);

	UPROPERTY(BlueprintCallable)
	FOnLayerListUpdated					OnLayerListUpdated;

private:

	void AddLayer(ULayerBase &Layer);

	void SetupLayerDependency(ULayerBase &Layer, const FJsonObject &DependencyObject);

	FTwinWorldContext				*m_TwinWorldContext = 0;

	TArray<ULayerBase*>				m_LayersToInitialize;

	UPROPERTY()
	TMap<int32, ULayerBase*>		m_Layers;

	UPROPERTY()
	TArray<ULayerBase*>				m_ActiveLayers;
	int32							m_nextLayerId = 1;

};

inline const TMap<int32, ULayerBase*>& ULayerManager::GetLayers() const
{
	return m_Layers;
}

inline ULayerBase* ULayerManager::GetLayer(int32 LayerId) const
{
	return m_Layers.Contains(LayerId) ? m_Layers[LayerId] : 0;
}
