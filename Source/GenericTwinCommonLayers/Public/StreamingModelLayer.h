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

#include "Streaming/TileStreamingLayer.h"
#include "Layers/LayerTypes.h"

#include "StreamingModelLayer.generated.h" 


DECLARE_LOG_CATEGORY_EXTERN(LogStreamingModelLayer, Log, All);

namespace GenericTwin {

class ILocationMapper;

}	//	namespace GenericTwin 

class UMaterialInstanceDynamic;
class AGeoReferencingSystem;

UCLASS()
class GENERICTWINCOMMONLAYERS_API UStreamingModelLayer	:	public UTileStreamingLayer
{
	GENERATED_BODY()

	typedef UTileStreamingLayer Super;

public:

	static const SLayerMetaData& GetLayerMetaData();
	static TSharedPtr<FJsonObject> GetDefaultConfiguration();
	static ULayerBase* Create(FTwinWorldContext &TwinWorldCtx, const TSharedPtr<FJsonObject> &Parameters);

	virtual ~UStreamingModelLayer();

	/*
	*	ULayerBase
	*/
	virtual void InitializeLayer(FTwinWorldContext &TwinWorldCtx) override;

	virtual void UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds) override;

	virtual void OnEndPlay(FTwinWorldContext& TwinWorldCtx) override;

	/*
	*	UTileLayerBase
	*/
	virtual TileBasePtr CreateTile(FTwinWorldContext &TwinWorldCtx, const GenericTwin::TTileId &TileId) override;


	FString LoadTileModel(const GenericTwin::TTileId &TileId);

private:

	UStreamingModelLayer();

	void ExtractParameters(const FJsonObject &Parameters, FTwinWorldContext &TwinWorldCtx);

	void ExtractModelParameters(const FJsonObject &ParametersObject);

	GenericTwin::ScenePtr LoadModel(const FString &TileUrl, const FVector2D &Origin);

	GenericTwin::TTileId MapLocationToTileId(const FVector& Location);

	FString								m_CacheSlot;

	FString								m_StreamingConfigFile;
	GenericTwin::ITileIndex				*m_TileIndex = 0;

	GenericTwin::ILocationMapper		*m_LocationMapper = 0;

	FVector								m_ImportTranslation;
	FRotator							m_ImportRotation;
	double								m_UniformScale;
	bool								m_FlipNormals = false;
	bool								m_FlipWindingOrder = false;
	FString								m_ModelMaterial;

};
