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

#include "Common/Caching/FeatureCache.h"
#include "GenericTwin.h"

#include "Kismet/KismetMathLibrary.h"

#include "concavehull.hpp"
#include <vector>

#include "Utils/GenericTwinTriangulator.h"

int64 UFeatureCache::AddStaticMesh(EFeatureType FeatureType, UStaticMeshComponent *MeshCmp, EFeatureGeometryType GeometryType)
{
	return AddStaticMeshWithCustomData(FeatureType, MeshCmp, GeometryType, {});
}

int64 UFeatureCache::AddStaticMeshWithCustomData(EFeatureType FeatureType, UStaticMeshComponent *MeshCmp, EFeatureGeometryType GeometryType, const TMap<FString, FString> &CustomData)
{
	if (MeshCmp->GetStaticMesh()->GetRenderData()->LODResources.Num() > 0)
	{
		FTransform transform = MeshCmp->GetComponentTransform();
		FPositionVertexBuffer* VertexBuffer = &MeshCmp->GetStaticMesh()->GetRenderData()->LODResources[0].VertexBuffers.PositionVertexBuffer;
		if (VertexBuffer)
		{
			AGenericTwin *genericTwin = AGenericTwin::GetInstance();
			FFeatureMesh featureMesh;
			const int32 VertexCount = VertexBuffer->GetNumVertices();
			for (int32 Index = 0; Index < VertexCount; Index++)
			{
				const FVector3f& vertex = VertexBuffer->VertexPosition(Index);
				const FVector WorldSpaceVertexLocation = transform.TransformPosition(FVector(vertex.X, vertex.Y, vertex.Z));
				featureMesh.Vertices.Add( genericTwin->ConvertEngineToGeoLocation(WorldSpaceVertexLocation));
			}

			//	todo: convert custom data
			AddFeature(FeatureType, featureMesh, GeometryType, {});
		}
	}

	return 0;
}

TFeatureId UFeatureCache::AddFeature(EFeatureType FeatureType, const FFeatureMesh &Mesh, EFeatureGeometryType GeometryType, const TCustomFeatureData &CustomData)
{
	FBox2d boundingRect = CalculateBoundingBox(Mesh);
	TFeatureId id = 0;

	switch(GeometryType)
	{
		case EFeatureGeometryType::MESH:
				id = AddFeature({FeatureType, Mesh, {}, boundingRect});
			break;

		case EFeatureGeometryType::AABB:
		case EFeatureGeometryType::OBB:
			{
				FBox box(Mesh.Vertices);

				box.Max.Z -= box.Min.Z;
				box.Min.Z = 0.0;

				FFeatureMesh aabbMesh;
				aabbMesh.Vertices.Add(box.Min);
				aabbMesh.Vertices.Add(FVector(box.Min.X, box.Max.Y, box.Min.Z));
				aabbMesh.Vertices.Add(FVector(box.Max.X, box.Max.Y, box.Min.Z));
				aabbMesh.Vertices.Add(FVector(box.Max.X, box.Min.Y, box.Min.Z));

				aabbMesh.Vertices.Add(FVector(box.Min.X, box.Min.Y, box.Max.Z));
				aabbMesh.Vertices.Add(FVector(box.Min.X, box.Max.Y, box.Max.Z));
				aabbMesh.Vertices.Add(FVector(box.Max.X, box.Max.Y, box.Max.Z));
				aabbMesh.Vertices.Add(FVector(box.Max.X, box.Min.Y, box.Max.Z));

				aabbMesh.Indices = {0,5,4, 0,1,5,1,2,6, 6,5,1,7,2,3, 7,6,2,4,7,3, 4,3,0,5,6,7, 7,4,5,0,2,1, 0,3,2};
				id = AddFeature({FeatureType, aabbMesh, {}, boundingRect});
			}
			break;

		case EFeatureGeometryType::PROXYMESH:
			{
				// build proxy mesh by treating originalk feature mesh as point cloud
				// project all points onto XY plane, calculate concave hull and build simple, closed mesh using this outline
				double minZ = TNumericLimits<double>::Max();
				double maxZ = -TNumericLimits<double>::Max();
				for(const FVector &v : Mesh.Vertices)
				{
					if(v.Z < minZ) minZ = v.Z;
					if(v.Z > maxZ) maxZ = v.Z;
				}
				maxZ -= minZ;
				minZ = 0.0;
				
				FFeatureMesh proxyMesh;

				const double chiFactor = 0.05;
				TArray<FVector2D> hull = CalculateConcaveHull(Mesh, chiFactor);	//	returns points in clockwise order
				if(!hull.IsEmpty())
				{
					for(int32 i = hull.Num() - 1; i >= 0; --i)
						proxyMesh.Vertices.Add(FVector(hull[i], minZ));
					for(int32 i = hull.Num() - 1; i >= 0; --i)
						proxyMesh.Vertices.Add(FVector(hull[i], maxZ));

					const int32 n = hull.Num();
					for(int32 i = 0; i < n; ++i)
					{
						proxyMesh.Indices.Add(i);	proxyMesh.Indices.Add((i + 1) % n);		proxyMesh.Indices.Add(((i + 1) % n) + n);
						proxyMesh.Indices.Add(((i + 1) % n) + n);	proxyMesh.Indices.Add(i + n);	proxyMesh.Indices.Add(i);
					}

					TArray<int32> indices = GenericTwin::Triangulator::Triangulate(hull, false);
					proxyMesh.Indices.Append(indices);

					indices = GenericTwin::Triangulator::Triangulate(hull, true);
					for(int32 i = 0; i < indices.Num(); i += 3)
					{
						proxyMesh.Indices.Add(indices[i] + hull.Num());
						proxyMesh.Indices.Add(indices[i + 1] + hull.Num());
						proxyMesh.Indices.Add(indices[i + 2] + hull.Num());
					}

					id = AddFeature({FeatureType, proxyMesh, {}, boundingRect});
				}
			}
			break;
	}

	return id;
}

TFeatureId UFeatureCache::AddFeature(const SCachedFeature &Feature)
{
	FScopeLock lock(&m_Mutex);

	m_CachedFeatures.Add(m_NextId, Feature);
	const TFeatureId id = m_NextId;
	m_NextId++;

	return id;

}

void UFeatureCache::RemoveFeature(TFeatureId FeatureId)
{
}

void UFeatureCache::RemoveFeatures(const TArray<TFeatureId> &FeatureId)
{
}

void UFeatureCache::RemoveFeeatures(const FBox2d &Rectangle, const TArray<EFeatureType> &FeatureTypes)
{
}

void UFeatureCache::ExtractFeatures(const FBox2d &Rectangle, const TArray<EFeatureType> &FeatureTypes, TArray<SCachedFeature> &Features)
{
	FBox2d area( ConvertToMercator(Rectangle.Min.X, Rectangle.Min.Y), ConvertToMercator(Rectangle.Max.X, Rectangle.Max.Y) );

	FScopeLock lock(&m_Mutex);
	for(const auto &item : m_CachedFeatures)
	{
		const SCachedFeature &feature = item.Value;
		if(area.Intersect(feature.BoundingRect))
		{
			Features.Add(feature);
		}
	}
}

FBox2d UFeatureCache::CalculateBoundingBox(const FFeatureMesh &FeatureMesh)
{
	FBox box3D(FeatureMesh.Vertices);

	FVector2D ll = ConvertToMercator(box3D.Min.X, box3D.Min.Y);
	FVector2D ur = ConvertToMercator(box3D.Max.X, box3D.Max.Y);

	return FBox2d(ll, ur);
}

FVector2D UFeatureCache::ConvertToMercator(double Longitude, double Latitude)
{
	const double Radius = 6335439.327 * 100.0;		//	WGS84 ellipsoid earth radius in Unreal Units
	const double Pi = UKismetMathLibrary::GetPI();

	const double lat = Pi * Latitude / 180.0;
	const double t = FMath::Tan(0.25 * Pi + 0.5 * lat);
	const double fac = FMath::Loge(t);

	// "normalize" w/ respect to earth radius
	const double convertedLon = Radius * Longitude / 180.0; 
	const double convertedLat = fac * Radius * Latitude / 90.0; 


	return FVector2D(convertedLon, convertedLat);
}

TArray<FVector2D> UFeatureCache::CalculateConcaveHull(const FFeatureMesh &FeatureMesh, double ChiFactor)
{
	std::vector<double> points;
	points.reserve(FeatureMesh.Vertices.Num() * 2);		//	2D coordinates
	for(const FVector &v : FeatureMesh.Vertices)
	{
		points.push_back(v.X);
		points.push_back(v.Y);
	}

	std::vector<double> hull = concavehull(gsl::span<double>(points), ChiFactor);

	TArray<FVector2D> concaveHull;
	for(size_t i = 0; i < hull.size(); i += 2)
	{
		concaveHull.Add(FVector2D(hull[i], hull[i + 1]));
	}

	return concaveHull;
}
