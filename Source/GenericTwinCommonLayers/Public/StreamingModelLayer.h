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

#include "Common/JobQueue/TJobQueue.h"

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

private:

	enum EType
	{
		None,
		Initialize,
		LoadTile
	};

	struct SRequest
	{
		SRequest(EType _Type)
			:	Type(_Type)
		{	}

		EType						Type;
	};

	struct SResult
	{
		SResult(EType _Type, bool _Success)
			:	Type(_Type)
			,	Success(_Success)
		{	}

		virtual ~SResult()
		{
		}

		EType		Type = EType::None;
		bool		Success = false;
	};

	struct SInitializeRequest	:	public SRequest
	{
		SInitializeRequest(const FString &_StreamingConfigFile)
			:	SRequest(EType::Initialize)
			,	StreamingConfigFile(_StreamingConfigFile)
		{	}

		FString						StreamingConfigFile;
	};

	struct SInitializeResult	:	public SResult
	{
		SInitializeResult(bool _Success)
			:	SResult(EType::Initialize, _Success)
		{	}
	};

	struct SLoadTileRequest	:	public SRequest
	{
		SLoadTileRequest(const GenericTwin::TTileId &_TileId, const FString &_Url, const FVector2D &_Origin)
			:	SRequest(EType::LoadTile)
			,	TileId(_TileId)
			,	Url(_Url)
			,	Origin(_Origin)
		{	}

		GenericTwin::TTileId		TileId;
		FString						Url;
		FVector2D					Origin;
	};

	struct SLoadTileResult	:	public SResult
	{
		SLoadTileResult(const GenericTwin::ScenePtr &_ScenePtr, const GenericTwin::TTileId &_TileId)
			:	SResult(EType::LoadTile, _ScenePtr.operator bool())
			,	TileId(_TileId)
			,	ScenePtr(_ScenePtr)
		{	}

		virtual ~SLoadTileResult()
		{
			ScenePtr.Reset();
		}

		GenericTwin::TTileId		TileId;
		GenericTwin::ScenePtr		ScenePtr;
	};

	typedef TJobQueue<TSharedPtr<SRequest>, TSharedPtr<SResult>>	TJobQueue;

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

	// virtual void OnVisibilityChanged(FTwinWorldContext& TwinWorldCtx) override;

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

	TSharedPtr<SResult> HandleRequest(TSharedPtr<SRequest> Request);

	GenericTwin::ScenePtr LoadModel(const FString &TileUrl, const FVector2D &Origin);

	GenericTwin::TTileId MapLocationToTileId(const FVector& Location);

	FString								m_CacheSlot;

	TJobQueue							m_JobQueue;

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
