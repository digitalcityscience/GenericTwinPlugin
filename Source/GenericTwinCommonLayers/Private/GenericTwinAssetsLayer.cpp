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


#include "GenericTwinAssetsLayer.h"

#include "TwinWorldContext.h"
#include "Common/Configuration/ConfigurationJsonBuilder.h"
#include "Utils/FilePathUtils.h"

#include "Common/Configuration/JsonParser.h"

#include "Utils/GenericTwinClassFinder.h"

#include "GeoReferencingSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"


DEFINE_LOG_CATEGORY(LogGenericTwinAssetsLayer);

const SLayerMetaData& UGenericTwinAssetsLayer::GetLayerMetaData()
{
	static SLayerMetaData layerMetaData =	{	TEXT("Assets")
											,	FText::FromString(TEXT("Assets"))
											,	FText::FromString(TEXT("Load Unreal assets"))
											,	ELayerContributionType::Visualisation
											,	ELayerCreationType::RuntimeMultiple
											};

	return layerMetaData;
}

TSharedPtr<FJsonObject> UGenericTwinAssetsLayer::GetDefaultConfiguration()
{
	static TSharedPtr<FJsonObject> defaultConfig;

	// if(!defaultConfig)
	// {
	// 	ConfigurationJsonBuilder jcb;
	// 	jcb.Begin();
	// 	jcb.Add(TEXT("model"), TEXT("string"), TEXT("Model Description File"), false);
	// 	defaultConfig = jcb.Finalize();
	// }

	return defaultConfig;
}

ULayerBase* UGenericTwinAssetsLayer::Create(FTwinWorldContext &TwinWorldCtx, const TSharedPtr<FJsonObject> &Parameters)
{
	UGenericTwinAssetsLayer *layer = NewObject<UGenericTwinAssetsLayer>(TwinWorldCtx.World, FName(), RF_Standalone);

	if(layer)
	{
		layer->OnConstruction(GetLayerMetaData(), ELayerPriority::Medium);
		layer->extractParameters(Parameters);
	}

	return layer;
}


UGenericTwinAssetsLayer::UGenericTwinAssetsLayer()
{
}

UGenericTwinAssetsLayer::~UGenericTwinAssetsLayer()
{
}

/*
		{	"asset_type":	"blueprint"
		,	"asset_reference":	"/WindTurbinesLibraryPlugin/Blueprints/BPA_WindTurbine"
		,	"import_transform":	{	"translation":	[0, 0, 0]
								,	"scale":	[10, 10, 10]
								,	"rotation":	[0, 0, 180]
								}
		}
*/
void UGenericTwinAssetsLayer::extractParameters(const TSharedPtr<FJsonObject> &Parameters)
{
	GenericTwin::FJsonParser parser(Parameters);

	TMap<FString, uint32> assetTypeMap =	{	{TEXT("blueprint"), static_cast<uint32> (EAssetType::Blueprint)}
											,	{TEXT("static_mesh"), static_cast<uint32> (EAssetType::StaticMesh)}
											};

	const int32 numAssets = parser.PushArray(TEXT("assets"));
	if(numAssets > 0)
	{
		for(int32 i = 0; i < numAssets; ++i)
		{
			SAssetData assetData;
			if	(	parser.ParseArrayValue	(	i
											,	{	{GenericTwin::EJsonDataType::Enumerator, TEXT("asset_type"), &assetData.AssetType, false, assetTypeMap}
												,	{GenericTwin::EJsonDataType::String, TEXT("asset_reference"), &assetData.AssetReference}
												,	{GenericTwin::EJsonDataType::Vector3, TEXT("location"), &assetData.Location}
												// ,	{EJsonDataType::Bool, TEXT("visible"), &assetData.Visible}
												}
											)
				)
			{
				FVector translation;
				FVector rotation;
				FVector scale;
				if	(	parser.ParseArrayObject	(	i
												,	TEXT("import_transform")
												,	{	{GenericTwin::EJsonDataType::Vector3, TEXT("translation"), &translation, true}
													,	{GenericTwin::EJsonDataType::Vector3, TEXT("rotation"), &rotation, true}
													,	{GenericTwin::EJsonDataType::Vector3, TEXT("scale"), &scale, true}
													}
												)
					)
				{
					assetData.ImportTransform.SetTranslation(translation);
				}
				m_Assets.Add(assetData);
			}
		}

		parser.PopArray();
	}
}


void UGenericTwinAssetsLayer::InitializeLayer(FTwinWorldContext &TwinWorldCtx)
{
}

void UGenericTwinAssetsLayer::UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds)
{
	if(m_curAssetIndex < m_Assets.Num())
	{
		const SAssetData &data = m_Assets[m_curAssetIndex];

		FVector location;
		TwinWorldCtx.GeoReferencingSystem->GeographicToEngine( FVector(data.Location.Y, data.Location.X, data.Location.Z), location );
		switch(data.AssetType)
		{
			case EAssetType::Blueprint:
				{
					UClass *assetClass = UGenericTwinClassFinder::FindClass(GetWorld(), data.AssetReference);
					AActor *actor = GetWorld()->SpawnActor<AActor>(assetClass);
					if(actor)
					{
						actor->SetActorLocation(location);
						m_Actors.Add(actor);
					}
				}
				break;
		}

		++m_curAssetIndex;
	}
}

void UGenericTwinAssetsLayer::OnVisibilityChanged(FTwinWorldContext& TwinWorldCtx)
{
	for(AActor *actor : m_Actors)
		actor->SetActorHiddenInGame(!m_isVisible);
}

void UGenericTwinAssetsLayer::OnEndPlay(FTwinWorldContext &TwinWorldCtx)
{
	for(AActor *actor : m_Actors)
	{
		actor->Destroy();
	}
	m_Actors.Empty();
}

