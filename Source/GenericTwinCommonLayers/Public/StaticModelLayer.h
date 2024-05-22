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

#include "Common/JobQueue/TJobQueue.h"
#include "Common/BaseTypes.h"
#include "Common/Material/MaterialMappingItem.h"

#include "StaticModelLayer.generated.h" 


DECLARE_LOG_CATEGORY_EXTERN(LogStaticModelLayer, Log, All);

class UMaterialInstanceDynamic;
class AGeoReferencingSystem;
class AGenericTwinCustomMeshActor;

UCLASS()
class GENERICTWINCOMMONLAYERS_API UStaticModelLayer	:	public ULayerBase
{
	GENERATED_BODY()

	struct SMaterialData
	{
		// 	muss generische, common struktur sein
		// 	material mapping als Core Funktionalität	->	MaterialMapper
	};

	struct SModelData
	{
		FString							ModelId;
		FString							FileName;

		FTransform						ImportTransform;
		FVector							GeoLocation;
		float							Orientation;

		bool							FlipWindingOrder = false;
		bool							FlipNormals = false;

		TMap<FString, GenericTwin::SMaterialMappingItem>	MaterialMappings;
	};

	struct SLoadModelRequest
	{
		AGeoReferencingSystem		*GeoReferencingSystem = 0;
		FString						FileName;

		SModelData					ModelData;
	};

	struct SLoadModelResult
	{
		GenericTwin::ScenePtr		ScenePtr;
	};

	typedef TJobQueue<SLoadModelRequest, SLoadModelResult*>	LoadJobQueue;

public:

	static const SLayerMetaData& GetLayerMetaData();
	static TSharedPtr<FJsonObject> GetDefaultConfiguration();
	static ULayerBase* Create(FTwinWorldContext &TwinWorldCtx, const TSharedPtr<FJsonObject> &Parameters);

	virtual ~UStaticModelLayer();

	virtual void InitializeLayer(FTwinWorldContext &TwinWorldCtx) override;

	virtual void UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds) override;

	virtual void OnVisibilityChanged(FTwinWorldContext& TwinWorldCtx) override;

	virtual void OnEndPlay(FTwinWorldContext& TwinWorldCtx) override;

private:

	UStaticModelLayer();

	void extractParameters(const FJsonObject &Parameters);

	void loadModelDescriptionFile(const FString &ModelDescriptionFile);

	SLoadModelResult* loadModel(const SLoadModelRequest &Request);

	void CreateModelActor(FTwinWorldContext &TwinWorldCtx, GenericTwin::ScenePtr ScenePtr);

	FString									m_BasePath;

	TArray<SModelData>						m_ModelsToLoad;
	int32									m_curModelToLoad = 0;

	bool									m_isModelPending = false;

	LoadJobQueue							*m_LoadJobQueue = 0;

	UPROPERTY()
	TArray<AGenericTwinCustomMeshActor*>	m_Actors;

};
