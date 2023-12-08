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


#include "StreamingModelLayer.h"
#include "StreamingModelTile.h"

#include "SpatialPartitioning/TileManager/TilingHelpers.h"

#include "Streaming/TileIndexes/FileSystemTileIndex.h"
#include "Utils/PathUtils.h"

#include "Import/ModelImporter.h"
#include "Import/ModelImportConfiguration.h"
#include "Common/Scene/Visitors/FlattenSceneVisitor.h"
#include "Common/Scene/Scene.h"
#include "Common/Files/SceneFile.h"

#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogStreamingModelLayer);

const SLayerMetaData& UStreamingModelLayer::GetLayerMetaData()
{
	static SLayerMetaData layerMetaData =	{	TEXT("StreamingModel")
											,	FText::FromString(TEXT("Streaming 3D Model Tiles"))
											,	FText::FromString(TEXT("Streaming 3D Model Tiles"))
											,	ELayerContributionType::Visualisation
											,	ELayerCreationType::RuntimeMultiple
											};

	return layerMetaData;
}

TSharedPtr<FJsonObject> UStreamingModelLayer::GetDefaultConfiguration()
{
	static TSharedPtr<FJsonObject> defaultConfig;

	if(!defaultConfig)
	{
		ConfigurationJsonBuilder jcb;
		jcb.Begin();
		jcb.Add(TEXT("model"), TEXT("string"), TEXT("Model Description File"), false);
		defaultConfig = jcb.Finalize();
	}

	return defaultConfig;
}

ULayerBase* UStreamingModelLayer::Create(FTwinWorldContext &TwinWorldCtx, const FJsonObject &Parameters)
{
	UStreamingModelLayer *layer = NewObject<UStreamingModelLayer>(TwinWorldCtx.World, FName(), RF_Standalone);

	if(layer)
	{
		layer->OnConstruction(GetLayerMetaData(), ELayerPriority::Medium);
		layer->ExtractParameters(Parameters, TwinWorldCtx);
	}

	return layer;
}

UStreamingModelLayer::UStreamingModelLayer()
	:	m_JobQueue(TEXT("StreamingModelLayer"))
{
}

UStreamingModelLayer::~UStreamingModelLayer()
{
}

void UStreamingModelLayer::ExtractParameters(const FJsonObject &Parameters, FTwinWorldContext &TwinWorldCtx)
{
	if(Parameters.HasTypedField<EJson::String>(TEXT("streaming_description")))
	{
		m_StreamingConfigFile = Parameters.GetStringField("streaming_description");
		m_TileIndex = new GenericTwin::FileSystemTileIndex(*(TwinWorldCtx.GeoReferencingSystem));

		m_DrawTileBorders &= Parameters.TryGetBoolField(TEXT("draw_tile_borders"), m_DrawTileBorders);

		m_CacheSlot = FString(TEXT("hamburg_terrain"));

		double viewDistance = Parameters.HasTypedField<EJson::Number>(TEXT("view_distance")) ? Parameters.GetNumberField(TEXT("view_distance")) : 4000.0f;
		viewDistance *= 100.0;

		int32 halfRectSize = Parameters.HasTypedField<EJson::Number>(TEXT("view_rect_half_size")) ? Parameters.GetNumberField(TEXT("view_rect_half_size")) : 3;

		UTileManager::SConfiguration config;
		config.MaxViewDistance = (Parameters.HasTypedField<EJson::Number>(TEXT("view_distance")) ? Parameters.GetNumberField(TEXT("view_distance")) : 4000.0f) * 100.0;
		config.MaxLoadDistance = Parameters.HasTypedField<EJson::Number>(TEXT("load_distance")) ? (Parameters.GetNumberField(TEXT("load_distance")) * 100.0) : config.MaxViewDistance;
		config.ViewRectHalfSize = Parameters.HasTypedField<EJson::Number>(TEXT("view_rect_half_size")) ? Parameters.GetNumberField(TEXT("view_rect_half_size")) : 3;
		config.MaxHeapMemoroyUsage = (Parameters.HasTypedField<EJson::Number>(TEXT("max_heap_memory_mb")) ? Parameters.GetNumberField(TEXT("max_heap_memory_mb")) : 1000) * 1024 * 1024;
		config.MaxGPUMemoryUsage = (Parameters.HasTypedField<EJson::Number>(TEXT("max_gpu_memory_mb")) ? Parameters.GetNumberField(TEXT("max_gpu_memory_mb")) : 2000) * 1024 * 1024;
		config.PurgeInterval = Parameters.HasTypedField<EJson::Number>(TEXT("purge_interval_s")) ? Parameters.GetNumberField(TEXT("purge_interval_s")) : 120;


		ConfigureTileManager(TEXT("StreamingModel"), config);
	}
}

void UStreamingModelLayer::ExtractModelParameters(const FJsonObject &ParametersObject)
{
	if (ParametersObject.HasTypedField<EJson::Boolean>(TEXT("flip_normals")))
		m_FlipNormals = ParametersObject.GetBoolField(TEXT("flip_normals"));

	if (ParametersObject.HasTypedField<EJson::Boolean>(TEXT("flip_winding_order")))
		m_FlipWindingOrder = ParametersObject.GetBoolField(TEXT("flip_winding_order"));

	if (ParametersObject.HasTypedField<EJson::String>(TEXT("material")))
		m_ModelMaterial = ParametersObject.GetStringField(TEXT("material"));

	if (ParametersObject.HasTypedField<EJson::Object>(TEXT("import_transform")))
	{
		const TSharedPtr<FJsonObject> importObject = ParametersObject.GetObjectField(TEXT("import_transform"));
		if (importObject->HasTypedField<EJson::Number>(TEXT("uniform_scale")))
			m_UniformScale = importObject->GetNumberField(TEXT("uniform_scale"));

		const TArray < TSharedPtr < FJsonValue > > *valArray = 0;
		if	(	importObject->TryGetArrayField(TEXT("translation"), valArray)
			&&	valArray
			&&	valArray->Num() > 2
			)
		{
			if((*valArray)[0]->Type == EJson::Number && (*valArray)[1]->Type == EJson::Number && (*valArray)[2]->Type == EJson::Number)
				m_ImportTranslation = FVector((*valArray)[0]->AsNumber(), (*valArray)[1]->AsNumber(), (*valArray)[2]->AsNumber());
		}

		if	(	importObject->TryGetArrayField(TEXT("rotation"), valArray)
			&&	valArray
			&&	valArray->Num() > 2
			)
		{
			if((*valArray)[0]->Type == EJson::Number && (*valArray)[1]->Type == EJson::Number && (*valArray)[2]->Type == EJson::Number)
				m_ImportRotation = FRotator((*valArray)[0]->AsNumber(), (*valArray)[1]->AsNumber(), (*valArray)[2]->AsNumber());
		}
	}
}

void UStreamingModelLayer::InitializeLayer(FTwinWorldContext &TwinWorldCtx)
{
	if(m_TileIndex)
	{
		SInitializeRequest *req = new SInitializeRequest(m_StreamingConfigFile);
		auto f = std::bind(&UStreamingModelLayer::HandleRequest, this, std::placeholders::_1);
		m_JobQueue.QueueRequest(TSharedPtr<SRequest>(req), f);
	}
}

void UStreamingModelLayer::UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds)
{
	Super::UpdateView(TwinWorldCtx, DeltaSeconds);

	TSharedPtr<SResult> resultPtr;
	if	(	m_JobQueue.GetResult(resultPtr)
		&&	resultPtr
		)
	{
		switch(resultPtr->Type)
		{
			case EType::Initialize:
				if (resultPtr->Success)
				{
					SInitializeResult *initResult = static_cast<SInitializeResult*> (resultPtr.Get());

					m_LocationMapper = new GenericTwin::BasicLocationMapper(m_TileIndex->GetReferenceLocation(), m_TileIndex->GetTileSize());
					if(m_LocationMapper)
					{
						SetupTiling(*m_LocationMapper, 5);
					}
				}
				break;

			case EType::LoadTile:
				{
					UE_LOG(LogStreamingModelLayer, Log, TEXT("Model tile received, adding to world"));
					SLoadTileResult *loadResult = static_cast<SLoadTileResult*> (resultPtr.Get()); 
					TileBasePtr tile = GetTile(loadResult->TileId);
					if (loadResult->Success)
					{
						UStreamingModelTile *modelTile = static_cast<UStreamingModelTile*> (tile.Get());

						FVector2D tileOrigin = m_TileIndex->GetTileOrigin(loadResult->TileId);
						FVector tileLocation;
						TwinWorldCtx.GeoReferencingSystem->ProjectedToEngine(FVector(tileOrigin, 0.0), tileLocation);

						UGenericTwinMaterialManager &matMgr = UGenericTwinMaterialManager::GetInstance();
						matMgr.SetOverrideMaterial(m_ModelMaterial);
						modelTile->AddToWorld(tileLocation, loadResult->ScenePtr);
						matMgr.ClearOverrideMaterial();
						UE_LOG(LogStreamingModelLayer, Log, TEXT("Done"));
					}
					else
					{
						if(tile)
						{
							tile->MarkAsEmpty();
						}
					}
				}
				break;
		}
	}
}

void UStreamingModelLayer::OnEndPlay(FTwinWorldContext& TwinWorldCtx)
{
	m_JobQueue.Stop();

	Super::OnEndPlay(TwinWorldCtx);
}

TileBasePtr UStreamingModelLayer::CreateTile(FTwinWorldContext &TwinWorldCtx, const GenericTwin::TTileId& TileId)
{
	UStreamingModelTile* tile = 0;

	if(m_TileIndex && m_TileIndex->HasTile(TileId))
	{
		tile = NewObject<UStreamingModelTile> (TwinWorldCtx.World, FName(), RF_Standalone);

		if(tile)
		{
			tile->SetLayer(this);
			tile->SetTileId(TileId);

			FVector location = TileId.ToLocation(m_TileIndex->GetReferenceLocation(), m_TileIndex->GetTileSize());
			tile->Initialize(location, m_TileIndex->GetTileSize());
		}
	}

	return TileBasePtr(tile);
}

FString UStreamingModelLayer::LoadTileModel(const GenericTwin::TTileId &TileId)
{
	SLoadTileRequest *request = new SLoadTileRequest(TileId, m_TileIndex->GetUrl(TileId), m_TileIndex->GetTileOrigin(TileId));
	if(request)
	{
		auto f = std::bind(&UStreamingModelLayer::HandleRequest, this, std::placeholders::_1);
		m_JobQueue.QueueRequest(TSharedPtr<SRequest>(request), f);
		return m_TileIndex->GetUrl(TileId);
	}
	return FString();
}

TSharedPtr<UStreamingModelLayer::SResult> UStreamingModelLayer::HandleRequest(TSharedPtr<SRequest> Request)
{
	SResult *result = 0;

	switch(Request->Type)
	{
		case EType::Initialize:
			{
				SInitializeRequest* initReq = static_cast<SInitializeRequest*> (Request.Get());
				const bool success = m_TileIndex->Initialize(GenericTwin::FPathUtils::MakeAbsolutPath(m_StreamingConfigFile, FPaths::LaunchDir()), std::bind(&UStreamingModelLayer::ExtractModelParameters, this, std::placeholders::_1));
				SInitializeResult *initRes = new SInitializeResult(success);
				result = initRes;
			}
			break;

		case EType::LoadTile:
			{
				SLoadTileRequest *request = static_cast<SLoadTileRequest*> (Request.Get());
				GenericTwin::ScenePtr scenePtr = LoadModel(request->Url, request->Origin);
				result = new SLoadTileResult(scenePtr, request->TileId);
			}
			break;
	}

	return TSharedPtr<SResult>(result);
}

GenericTwin::ScenePtr UStreamingModelLayer::LoadModel(const FString &ModelUrl, const FVector2D &Origin)
{
	UE_LOG(LogStreamingModelLayer, Log, TEXT("Starting to load model %s"), *(ModelUrl));

	GenericTwin::SModelImportConfiguration importConfig;

	importConfig.FlipWindingOrder = m_FlipWindingOrder;
	importConfig.FlipNormals = m_FlipNormals;
	importConfig.LoadTextures = true;

	GenericTwin::ScenePtr scenePtr = GenericTwin::ModelImporter::ImportModel(ModelUrl, importConfig);

	if(scenePtr)
	{
		UE_LOG(LogStreamingModelLayer, Log, TEXT("Model %s successfully loaded"), *(ModelUrl));
		FTransform transform( m_ImportRotation, FVector(-Origin.X, -Origin.Y, 0.0) + m_ImportTranslation, FVector(m_UniformScale) );
		GenericTwin::FlattenSceneVisitor fsVisitor(transform);
		scenePtr->Visit(fsVisitor);

		scenePtr.Reset();
		scenePtr = GenericTwin::ScenePtr(fsVisitor.GetFlattenedScene());

		GenericTwin::FSceneFile sceneFile;
		sceneFile.SaveSceneToCache(scenePtr, ModelUrl, m_CacheSlot);

	}
	else
	{
		UE_LOG(LogStreamingModelLayer, Warning, TEXT("Failed to load model %s"), *(ModelUrl));
	}

	return scenePtr;
}

GenericTwin::TTileId UStreamingModelLayer::MapLocationToTileId(const FVector& Location)
{
	return m_TileIndex ? GenericTwin::TTileId::FromLocation(Location, m_TileIndex->GetReferenceLocation(), m_TileIndex->GetTileSize()) : GenericTwin::TTileId();
}
