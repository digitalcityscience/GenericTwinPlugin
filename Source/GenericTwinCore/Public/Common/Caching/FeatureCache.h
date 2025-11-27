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

#include "Common/Base/SingletonManager.h"
#include "HAL/CriticalSection.h"
#include "FeatureCache.generated.h"


typedef int64 TFeatureId;
typedef TVariant<bool, int32, int64, float, double, FVector, FString> TFeatureDataItem;
typedef TMap<FString, TFeatureDataItem > TCustomFeatureData;

USTRUCT(BlueprintType)
struct FFeatureMesh
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadWrite, Category=Default)
	TArray<FVector>		Vertices;

	UPROPERTY(BlueprintReadWrite, Category=Default)
	TArray<int32>		Indices;				//	counter clock wise

};

UENUM(BlueprintType)
enum class EFeatureType : uint8
{
	BUILDING				UMETA(DisplayName = "Building"),
	TREE					UMETA(DisplayName = "Tree"),
};

UENUM(BlueprintType)
enum class EFeatureGeometryType : uint8
{
	MESH				UMETA(DisplayName = "Mesh"),
	AABB				UMETA(DisplayName = "AxisAlignedBoundingBox"),
	OBB					UMETA(DisplayName = "OrientedBoundingBox"),
	PROXYMESH			UMETA(DisplayName = "ProxyMesh"),
};

UCLASS()
class GENERICTWINCORE_API UFeatureCache	:	public UObject
										,	public GenericTwin::TSingletonManager<UFeatureCache>
{
	GENERATED_BODY()

	friend class GenericTwin::TSingletonManager<class UFeatureCache>;

public:

	struct SCachedFeature
	{
		EFeatureType				FeatureType;
		FFeatureMesh				FeatureMesh;
		TCustomFeatureData			CustomData;

		FBox2d						BoundingRect;
	};

public:

	UFUNCTION(BlueprintCallable, Category="GenericTwin")
	int64 AddStaticMesh(EFeatureType FeatureType, UStaticMeshComponent *MeshCmp, EFeatureGeometryType GeometryType);

	UFUNCTION(BlueprintCallable, Category="GenericTwin")
	int64 AddStaticMeshWithCustomData(EFeatureType FeatureType, UStaticMeshComponent *MeshCmp, EFeatureGeometryType GeometryType, const TMap<FString, FString> &CustomData);

	TFeatureId AddFeature(EFeatureType FeatureType, const FFeatureMesh &Mesh, EFeatureGeometryType GeometryType, const TCustomFeatureData &CustomData = {});

	void RemoveFeature(TFeatureId FeatureId);
	void RemoveFeatures(const TArray<TFeatureId> &FeatureId);
	void RemoveFeeatures(const FBox2d &Rectangle, const TArray<EFeatureType> &FeatureTypes);

	void ExtractFeatures(const FBox2d &Rectangle, const TArray<EFeatureType> &FeatureTypes, TArray<SCachedFeature> &Features);

protected:

	virtual void Shutdown() override
		{	}

private:

	TFeatureId AddFeature(const SCachedFeature &Feature);

	FBox2d CalculateBoundingBox(const FFeatureMesh &FeatureMesh);

	TArray<FVector2D> CalculateConcaveHull(const FFeatureMesh &FeatureMesh, double ChiFactor);

	FVector2D ConvertToMercator(double Longitude, double Latitude);

	FCriticalSection							m_Mutex;

	TMap<TFeatureId, SCachedFeature>			m_CachedFeatures;
	TFeatureId									m_NextId = 1;
};

