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


#include "StaticModelLayer.h"

#include "TwinWorldContext.h"
#include "Common/Configuration/ConfigurationJsonBuilder.h"
#include "Common/Scene/Scene.h"

#include "Actors/GenericTwinCustomMeshActor.h"

#include "Import/ModelImporter.h"
#include "Import/ModelImportConfiguration.h"
#include "Common/Scene/Visitors/CalcSceneBoundingsVisitor.h"
#include "Common/Scene/Visitors/FlattenSceneVisitor.h"

#include "Common/Files/SceneFile.h"

#include "Common/Material/GenericTwinMaterialManager.h"

#include "Utils/PathUtils.h"

#include "GeoReferencingSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY(LogStaticModelLayer);

const SLayerMetaData& UStaticModelLayer::GetLayerMetaData()
{
	static SLayerMetaData layerMetaData =	{	TEXT("StaticModel")
											,	FText::FromString(TEXT("Static 3D Model"))
											,	FText::FromString(TEXT("Provides static 3D model"))
											,	ELayerContributionType::Visualisation
											,	ELayerCreationType::RuntimeMultiple
											};

	return layerMetaData;
}

TSharedPtr<FJsonObject> UStaticModelLayer::GetDefaultConfiguration()
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

ULayerBase* UStaticModelLayer::Create(FTwinWorldContext &TwinWorldCtx, const TSharedPtr<FJsonObject> &Parameters)
{
	UStaticModelLayer *layer = NewObject<UStaticModelLayer>(TwinWorldCtx.World, FName(), RF_Standalone);

	if(layer)
	{
		layer->OnConstruction(GetLayerMetaData(), ELayerPriority::Medium);
		layer->extractParameters(*Parameters);
	}

	return layer;
}


UStaticModelLayer::UStaticModelLayer()
{
}

UStaticModelLayer::~UStaticModelLayer()
{
}

void UStaticModelLayer::extractParameters(const FJsonObject &Parameters)
{
	if(Parameters.HasTypedField<EJson::String>(TEXT("model")))
	{
		FString mdf = Parameters.GetStringField("model");
		loadModelDescriptionFile(GenericTwin::FPathUtils::MakeAbsolutPath(mdf, FPaths::LaunchDir()));
	}
	else if	(	Parameters.HasTypedField<EJson::String>(TEXT("base_path"))
			&&	Parameters.HasTypedField<EJson::Array>(TEXT("models"))
			)
	{
		m_BasePath = Parameters.GetStringField("base_path");

		const TArray < TSharedPtr < FJsonValue > > *models = 0;
		if	(	Parameters.TryGetArrayField(TEXT("models"), models)
			&&	models
			)
		{
			for(const auto &model : (*models))
			{
				if(model->Type == EJson::String)
				{
					FString modelStr = model->AsString();
					FString mdf = FPaths::Combine(m_BasePath, modelStr);
					loadModelDescriptionFile(GenericTwin::FPathUtils::MakeAbsolutPath(mdf, FPaths::LaunchDir()));
				}
			}
		}
	}
}

void UStaticModelLayer::loadModelDescriptionFile(const FString &ModelDescriptionFile)
{
	FString jsonStr;
	if (FFileHelper::LoadFileToString(jsonStr, *ModelDescriptionFile))
	{
		UE_LOG(LogStaticModelLayer, Log, TEXT("Model description file %s successfully loaded"), *ModelDescriptionFile);

		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonStr);
		TSharedPtr<FJsonObject> jsonObject;
		if (FJsonSerializer::Deserialize(reader, jsonObject))
		{
			if	(	jsonObject->HasTypedField<EJson::String>(TEXT("model"))
				&&	jsonObject->HasTypedField<EJson::Array>(TEXT("location"))
				)
			{
				SModelData modelData;

				modelData.FileName = jsonObject->GetStringField("model");
				if(FPaths::IsRelative(modelData.FileName))
				{
					FString path;
					FString fileName;
					FString extension;
					FPaths::Split(ModelDescriptionFile, path, fileName, extension);
					modelData.FileName = FPaths::Combine(path, modelData.FileName);
				}

				const TArray<TSharedPtr<FJsonValue>> arrayVals = jsonObject->GetArrayField("location");
				modelData.GeoLocation = FVector::ZeroVector;
				if(arrayVals.Num() > 1)
				{
					modelData.GeoLocation.X = arrayVals[0]->AsNumber();
					modelData.GeoLocation.Y = arrayVals[1]->AsNumber();
					if(arrayVals.Num() > 2)
						modelData.GeoLocation.Z = arrayVals[2]->AsNumber();
					
					if(jsonObject->HasTypedField<EJson::Number>(TEXT("orientation")))
						modelData.Orientation = jsonObject->GetNumberField("orientation");

					if(jsonObject->HasTypedField<EJson::Object>(TEXT("import_transform")))
					{
						TSharedPtr<FJsonObject> itObject = jsonObject->GetObjectField("import_transform");

						if(itObject->HasTypedField<EJson::Array>(TEXT("translation")))
						{
							const TArray<TSharedPtr<FJsonValue>> vals = itObject->GetArrayField("translation");
							if(vals.Num() > 2)
							{
								modelData.ImportTransform.SetTranslation(FVector(vals[0]->AsNumber(), vals[1]->AsNumber(), vals[2]->AsNumber()));
							}
						}

						if(itObject->HasTypedField<EJson::Array>(TEXT("rotation")))
						{
							const TArray<TSharedPtr<FJsonValue>> vals = itObject->GetArrayField("rotation");
							if(vals.Num() > 2)
							{
								modelData.ImportTransform.SetRotation(FRotator(vals[0]->AsNumber(), vals[1]->AsNumber(), vals[2]->AsNumber()).Quaternion());
							}
						}

						if(itObject->HasTypedField<EJson::Array>(TEXT("scale")))
						{
							const TArray<TSharedPtr<FJsonValue>> vals = itObject->GetArrayField("scale");
							if(vals.Num() > 2)
							{
								modelData.ImportTransform.SetScale3D(FVector(vals[0]->AsNumber(), vals[1]->AsNumber(), vals[2]->AsNumber()));
							}
						}

					}

					bool tmpBool = false;
					if(jsonObject->TryGetBoolField(TEXT("flip_winding_order"), tmpBool))
						modelData.FlipWindingOrder = tmpBool;
					if(jsonObject->TryGetBoolField(TEXT("flip_normals"), tmpBool))
						modelData.FlipNormals = tmpBool;

					const TArray < TSharedPtr < FJsonValue > > *materials = 0;
					if	(	jsonObject->TryGetArrayField(TEXT("materials"), materials)
						&&	materials
						)
					{
						UGenericTwinMaterialManager &matMgr = UGenericTwinMaterialManager::GetInstance();
						for(const auto &mapping : (*materials))
						{
							GenericTwin::SMaterialMappingItem matItem;
							if	(	mapping->Type == EJson::Object
								&& matMgr.ParseMaterialItem(mapping->AsObject(), matItem)
								&&	matItem.MaterialName.IsEmpty() == false
								)
							{
								modelData.MaterialMappings.Add(matItem.MaterialName, matItem);
							}
						}
					}

					m_ModelsToLoad.Add(modelData);
				}
			}
		}
		else
		{
			UE_LOG(LogStaticModelLayer, Warning, TEXT("Error parsing %s"), *ModelDescriptionFile);
		}
	}
	else
	{
		UE_LOG(LogStaticModelLayer, Warning, TEXT("Failed to load odel description file %s"), *ModelDescriptionFile);
	}
}


void UStaticModelLayer::InitializeLayer(FTwinWorldContext &TwinWorldCtx)
{
	m_curModelToLoad = 0;
	m_isModelPending = false;
	m_LoadJobQueue = new LoadJobQueue(TEXT("StaticModelLoader"));

	m_curModelToLoad = 0;
}

void UStaticModelLayer::UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds)
{
	if(m_isModelPending)
	{
		SLoadModelResult *result = 0;

		if (m_LoadJobQueue->GetResult(result))
		{
			if(result)
			{
				if(result->ScenePtr)
				{
					CreateModelActor(TwinWorldCtx, result->ScenePtr);
				}

				delete result;
			}

			m_isModelPending = false;
			++m_curModelToLoad;
		}
	}
	else if(m_curModelToLoad < m_ModelsToLoad.Num())
	{
		const FString fileName = m_ModelsToLoad[m_curModelToLoad].FileName;

		SLoadModelRequest request = { TwinWorldCtx.GeoReferencingSystem, fileName, m_ModelsToLoad[m_curModelToLoad]};
		m_isModelPending = true;
		m_LoadJobQueue->QueueRequest( request, [this](const SLoadModelRequest &Request) -> SLoadModelResult* {
			return loadModel(Request);
		});
	}

}

void UStaticModelLayer::OnVisibilityChanged(FTwinWorldContext& TwinWorldCtx)
{
	for(AActor *actor : m_Actors)
		actor->SetActorHiddenInGame(!m_isVisible);
}

void UStaticModelLayer::OnEndPlay(FTwinWorldContext &TwinWorldCtx)
{
	if(m_LoadJobQueue)
	{
		m_LoadJobQueue->Stop();
		delete m_LoadJobQueue;
	}

	for(AGenericTwinCustomMeshActor* a : m_Actors)
	{
		a->Release();
	}
	m_Actors.Empty();
}

UStaticModelLayer::SLoadModelResult* UStaticModelLayer::loadModel(const UStaticModelLayer::SLoadModelRequest &Request)
{
	SLoadModelResult *result = new SLoadModelResult;
	if (result)
	{
		GenericTwin::SModelImportConfiguration importConfig;

		importConfig.ImportTransform = Request.ModelData.ImportTransform;
		importConfig.FlipWindingOrder = Request.ModelData.FlipWindingOrder;
		importConfig.FlipNormals = Request.ModelData.FlipNormals;

		GenericTwin::ScenePtr scenePtr = GenericTwin::ModelImporter::ImportModel(Request.FileName, importConfig);

		if(scenePtr)
		{
			GenericTwin::FSceneFile sceneFile;
			sceneFile.SaveSceneToCache(scenePtr, Request.FileName, TEXT("static_model"));

			UE_LOG(LogStaticModelLayer, Log, TEXT("Model %s successfully loaded"), *(Request.FileName));
			GenericTwin::FlattenSceneVisitor fsVisitor(Request.ModelData.ImportTransform);
			scenePtr->Visit(fsVisitor);

			scenePtr.Reset();
			scenePtr = GenericTwin::ScenePtr(fsVisitor.GetFlattenedScene());

			GenericTwin::CalcSceneBoundingsVisitor sbVisitor;
			scenePtr->Visit(sbVisitor);

			const FVector &minExtent = sbVisitor.GetMinExtent();
			const FVector &maxExtent = sbVisitor.GetMaxExtent();

			FVector ctrOffset(0.5 * (minExtent.X + maxExtent.X), 0.5 * (minExtent.Y + maxExtent.Y), -minExtent.Z);

			for(int32 i = 0; i < scenePtr->GetMeshCount(); ++i)
			{
				GenericTwin::SSceneMesh *mesh = scenePtr->GetMesh(i);
				for(FVector &p : mesh->Vertices)
					p -= ctrOffset;
			}

			result->ScenePtr = scenePtr;

		}
		else
		{
			UE_LOG(LogStaticModelLayer, Warning, TEXT("Failed to load model %s"), *(Request.FileName));
		}
	}
	return result;
}

void UStaticModelLayer::CreateModelActor(FTwinWorldContext &TwinWorldCtx, GenericTwin::ScenePtr ScenePtr)
{
	const SModelData &mdlData = m_ModelsToLoad[m_curModelToLoad];
	const FVector &geoLoc = mdlData.GeoLocation;
	FCartesianCoordinates projectedCoords;
	TwinWorldCtx.GeoReferencingSystem->GeographicToProjected(FGeographicCoordinates(geoLoc.X, geoLoc.Y, geoLoc.Z), projectedCoords);
	FVector actorLocation;
	TwinWorldCtx.GeoReferencingSystem->ProjectedToEngine(projectedCoords, actorLocation);

	AGenericTwinCustomMeshActor *modelActor = TwinWorldCtx.World->SpawnActor<AGenericTwinCustomMeshActor>(actorLocation, FRotator(0.0, mdlData.Orientation, 0.0), FActorSpawnParameters());

	if(modelActor)
	{
		UGenericTwinMaterialManager& matMgr = UGenericTwinMaterialManager::GetInstance();
		matMgr.SetExactMaterialMapping(mdlData.MaterialMappings);
		modelActor->SetFlattenedScene(ScenePtr);
		m_Actors.Add(modelActor);
		matMgr.ClearExactMaterialMapping();
	}
}
