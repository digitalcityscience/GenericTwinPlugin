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

#include "Common/JobQueue/GenericTwinJobBase.h"
#include "Common/JobQueue/GenericTwinJobDispatcher.h"

#include "SpatialPartitioning/TileManager/TilingHelpers.h"

#include "Streaming/TileIndexes/FileSystemTileIndex.h"
#include "Utils/FilePathUtils.h"
#include "Utils/GenericTwinCoreUtils.h"

#include "Import/ModelImporter.h"
#include "Import/ModelImportConfiguration.h"
#include "Common/Scene/Visitors/FlattenSceneVisitor.h"
#include "Common/Scene/Scene.h"
#include "Common/Files/SceneFile.h"

#include "Misc/Paths.h"
#include "GenericPlatform/GenericPlatformMisc.h"

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

ULayerBase* UStreamingModelLayer::Create(FTwinWorldContext &TwinWorldCtx, const TSharedPtr<FJsonObject> &Parameters)
{
	UStreamingModelLayer *layer = NewObject<UStreamingModelLayer>(TwinWorldCtx.World, FName(), RF_Standalone);

	if(layer)
	{
		layer->OnConstruction(GetLayerMetaData(), ELayerPriority::Medium);
		layer->ExtractParameters(*Parameters, TwinWorldCtx);
	}

	return layer;
}

UStreamingModelLayer::UStreamingModelLayer()
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
	class InitializeLayerJob	:	public GenericTwin::JobBase
	{
	public:

		InitializeLayerJob(UStreamingModelLayer &Layer, const FString &ConfigFileName)
			:	JobBase(GenericTwin::EJobType::Background)
			,	m_Layer(Layer)
			,	m_ConfigFileName(ConfigFileName)
		{
		}

		virtual GenericTwin::TJobBasePtr Execute()
		{
			const FString baseDir = GenericTwin::CoreUtils::IsRunningFromEditor() ? FPaths::ProjectDir() : FPaths::LaunchDir();
			m_Success = m_Layer.m_TileIndex->Initialize(GenericTwin::FPathUtils::MakeAbsolutPath(m_ConfigFileName, baseDir), std::bind(&UStreamingModelLayer::ExtractModelParameters, &m_Layer, std::placeholders::_1));
			return GenericTwin::TJobBasePtr();
		}

		virtual void OnJobFinished()
		{
			if(m_Success)
			{
				m_Layer.m_LocationMapper = new GenericTwin::BasicLocationMapper(m_Layer.m_TileIndex->GetReferenceLocation(), m_Layer.m_TileIndex->GetTileSize());
				if(m_Layer.m_LocationMapper)
				{
					m_Layer.SetupTiling(*(m_Layer.m_LocationMapper), 5);
				}
			}
		}

	private:
		UStreamingModelLayer		&m_Layer;
		const FString				&m_ConfigFileName;
		bool						m_Success = false;
	};

	if(m_TileIndex)
	{
		TSharedPtr<InitializeLayerJob> jobPtr(new InitializeLayerJob(*this, m_StreamingConfigFile));
		FGenericTwinJobDispatcher::DispatchJob(jobPtr);
	}
}

void UStreamingModelLayer::UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds)
{
	Super::UpdateView(TwinWorldCtx, DeltaSeconds);

}

void UStreamingModelLayer::OnEndPlay(FTwinWorldContext& TwinWorldCtx)
{
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
	class LoadModelJob	:	public GenericTwin::JobBase
	{
	public:

		LoadModelJob(UStreamingModelLayer &Layer, const GenericTwin::TTileId &TileId, const FString &Url, const FVector2D &Origin)
			:	JobBase(GenericTwin::EJobType::Background)
			,	m_Layer(Layer)
			,	m_TileId(TileId)
			,	m_Url(Url)
			,	m_Origin(Origin)
		{
		}

		virtual GenericTwin::TJobBasePtr Execute() override
		{
			m_ScenePtr = m_Layer.LoadModel(m_Url, m_Origin);
			return GenericTwin::TJobBasePtr();
		}

		virtual void OnJobFinished()
		{
			UE_LOG(LogStreamingModelLayer, Log, TEXT("Model tile received, adding to world"));
			TileBasePtr tile = m_Layer.GetTile(m_TileId);
			if (m_ScenePtr.IsValid())
			{
				m_ScenePtr->ApplyMaterial(m_Layer.m_ModelMaterial);

				UStreamingModelTile *modelTile = static_cast<UStreamingModelTile*> (tile.Get());
				const auto uploadMode = modelTile->AddToWorld(tile->GetWorldLocation());
				const auto jobType = uploadMode == GenericTwin::IStaticMeshComponent::EUploadMode::GameThread ? GenericTwin::EJobType::GameThread : GenericTwin::EJobType::Background;

				UE_LOG(LogStreamingModelLayer, Log, TEXT("Done"));

				GenericTwin::ScenePtr scenePtr = m_ScenePtr;
				FGenericTwinJobDispatcher::Dispatch<void()>([modelTile, scenePtr]() {modelTile->SetScene(scenePtr); }, jobType);
			}
			else
			{
				if(tile)
				{
					tile->MarkAsEmpty();
				}
			}
		}

	private:
		UStreamingModelLayer		&m_Layer;
		GenericTwin::TTileId		m_TileId;
		FString						m_Url;
		FVector2D					m_Origin;

		GenericTwin::ScenePtr		m_ScenePtr;
	};


	TSharedPtr<LoadModelJob> jobPtr(new LoadModelJob(*this, TileId, m_TileIndex->GetUrl(TileId), m_TileIndex->GetTileOrigin(TileId)));
	FGenericTwinJobDispatcher::DispatchJob(jobPtr);

	return FString();
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
