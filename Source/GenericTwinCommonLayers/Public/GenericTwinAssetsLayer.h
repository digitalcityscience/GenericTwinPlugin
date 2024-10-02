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

#include "Layers/LayerBase.h"
#include "Layers/LayerTypes.h"

#include "GenericTwinAssetsLayer.generated.h" 


DECLARE_LOG_CATEGORY_EXTERN(LogGenericTwinAssetsLayer, Log, All);

class AGeoReferencingSystem;

UCLASS()
class GENERICTWINCOMMONLAYERS_API UGenericTwinAssetsLayer	:	public ULayerBase
{
	GENERATED_BODY()

private:

	enum EAssetType
	{
		Invalid,
		Blueprint,
		StaticMesh
	};

	struct SAssetData
	{
		EAssetType				AssetType = EAssetType::Invalid;
		FString					AssetReference;
		FVector					Location;
		FTransform				ImportTransform;
	};

public:

	static const SLayerMetaData& GetLayerMetaData();
	static TSharedPtr<FJsonObject> GetDefaultConfiguration();
	static ULayerBase* Create(FTwinWorldContext &TwinWorldCtx, const TSharedPtr<FJsonObject> &Parameters);

	virtual ~UGenericTwinAssetsLayer();

	virtual void InitializeLayer(FTwinWorldContext &TwinWorldCtx) override;

	virtual void UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds) override;

	virtual void OnVisibilityChanged(FTwinWorldContext& TwinWorldCtx) override;

	virtual void OnEndPlay(FTwinWorldContext& TwinWorldCtx) override;

private:

	UGenericTwinAssetsLayer();

	void extractParameters(const TSharedPtr<FJsonObject> &Parameters);

	TArray<SAssetData>						m_Assets;
	int32									m_curAssetIndex = 0;

	UPROPERTY()
	TArray<AActor*>							m_Actors;

};
