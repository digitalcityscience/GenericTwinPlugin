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

#include "Components/GenericTwinStaticRTMComponent.h"
#include "Common/BaseTypes.h"

#include "RealtimeMeshSimple.h"
#include "Mesh/RealtimeMeshBuilder.h"
#include "Mesh/RealtimeMeshBlueprintMeshBuilder.h"

UGenericTwinStaticRTMComponent::UGenericTwinStaticRTMComponent()
{

}

UGenericTwinStaticRTMComponent::~UGenericTwinStaticRTMComponent()
{

}

void UGenericTwinStaticRTMComponent::Construct(AActor *Actor, EUploadMode UploadMode)
{
    m_UploadMode = UploadMode;
	URealtimeMeshSimple* NewMesh =  Actor->CreateDefaultSubobject<URealtimeMeshSimple>(TEXT("RTMMeshSimple"));
	SetRealtimeMesh(NewMesh);
}

void UGenericTwinStaticRTMComponent::Initialize(GenericTwin::IStaticMeshComponent::EUploadMode UploadMode)
{
    m_UploadMode = UploadMode;
	if(m_Mesh == 0)
	{
		m_Mesh = InitializeRealtimeMesh<URealtimeMeshSimple>();
	}
}

void UGenericTwinStaticRTMComponent::Clear()
{
}

int32 UGenericTwinStaticRTMComponent::AddMeshSection(const GenericTwin::SMeshSection &MeshSection, UMaterialInterface *Material)
{
	if(m_Mesh)
	{
        SMeshSectionData msd = {MeshSection.Indices, MeshSection.Vertices };

		if(MeshSection.Normals.Num() == MeshSection.Vertices.Num())
		{
			msd.Normals = &MeshSection.Normals;
		}
		if(MeshSection.UV0.Num() == MeshSection.Vertices.Num())
		{
			msd.UV0 = &MeshSection.UV0;
		}
		if(MeshSection.UV1.Num() == MeshSection.Vertices.Num())
		{
			msd.UV1 = &MeshSection.UV1;
		}

		if(msd.UV0 != nullptr && msd.UV1 != nullptr)
			BuildMeshSection<uint32, 2>(msd);
		else if(msd.UV0 != nullptr)
			BuildMeshSection<uint32, 1>(msd);
		else
			BuildMeshSection<uint32, 0>(msd);


		// BuildMeshSection(msd);

		const int32 sectionIndex = m_numSections;
		++m_numSections;

		SetMaterial(sectionIndex, Material);

		return sectionIndex;
	}
    return -1;
}

int32 UGenericTwinStaticRTMComponent::AddMeshSection(const TArray<int32> &Indices, const TArray<FVector> &Vertices, const TArray<FVector> *Normals, const TArray<FVector2D> *UV0, const TArray<FVector2D> *UV1, const TArray<FLinearColor> *Colors, UMaterialInterface *Material)
{
	if(m_Mesh)
	{
		m_Mesh->SetupMaterialSlot(m_numSections, FName(TEXT("MaterialSlot_") + FString::FromInt(m_numSections)), Material);

        SMeshSectionData msd = {Indices, Vertices, Normals, UV0, UV1};

		if(UV0 != nullptr && UV1 != nullptr)
			BuildMeshSection<uint32, 2>(msd);
		else if(UV0 != nullptr)
			BuildMeshSection<uint32, 1>(msd);
		else
			BuildMeshSection<uint32, 0>(msd);

		const int32 sectionIndex = m_numSections;
		++m_numSections;

		// SetMaterial(sectionIndex, Material);

		return sectionIndex;
	}
    return -1;
}

int32 UGenericTwinStaticRTMComponent::AddMeshSections(const TArray<const GenericTwin::SMeshSection*> &MeshSections, const TArray<UMaterialInterface*> &Materials)
{
    return -1;
}

#define USE_PG


template<typename INDEXTYPE, int32 NumTexCoords>
void UGenericTwinStaticRTMComponent::BuildMeshSection(const SMeshSectionData &MeshSectionData)
{
	FRealtimeMeshStreamSet StreamSet;

	// TRealtimeMeshBuilderLocal<INDEXTYPE, FPackedNormal, FVector2DHalf, NumTexCoords> *builderPtr = new TRealtimeMeshBuilderLocal<INDEXTYPE, FPackedNormal, FVector2DHalf, NumTexCoords>(StreamSet);
	// TRealtimeMeshBuilderLocal<INDEXTYPE, FPackedNormal, FVector2DHalf, NumTexCoords> &Builder = *builderPtr;

	TRealtimeMeshBuilderLocal<INDEXTYPE, FPackedNormal, FVector2DHalf, NumTexCoords> Builder(StreamSet);

    if(MeshSectionData.Normals != nullptr)
    {
    	Builder.EnableTangents();
    }

	if(NumTexCoords > 0 && MeshSectionData.UV0 != nullptr)
	{
		Builder.EnableTexCoords(NumTexCoords);
	}

#ifdef USE_PG
	Builder.EnablePolyGroups();
#endif

	for(int32 i = 0; i < MeshSectionData.Vertices.Num(); ++i)
	{
		auto vertexBuilder = Builder.AddVertex(FVector3f(MeshSectionData.Vertices[i]));
		if (MeshSectionData.Normals != nullptr)
		{
			const FVector& n = MeshSectionData.Normals->operator[] (i);
			vertexBuilder.SetNormal(FVector3f(n.X, n.Y, n.Z));
		}

		if(NumTexCoords == 2)
		{
			vertexBuilder.SetTexCoord(FVector2f((*MeshSectionData.UV0)[i])).SetTexCoord(1, FVector2f((*MeshSectionData.UV1)[i]));
		}
		else if(NumTexCoords == 1)
		{
			vertexBuilder.SetTexCoord(FVector2f((*MeshSectionData.UV0)[i]));
		}

        // if(MeshSectionData.UV0 != 0)
		// {
		// 	if(NumTexCoords > 0)
		// 		vertexBuilder.SetTexCoord(FVector2f((*MeshSectionData.UV0)[i]));
		// 	// if(MeshSectionData.UV1 != 0)
		// 	// {
		// 	// 	vertexBuilder.SetTexCoord(1, FVector2f((*MeshSectionData.UV1)[i]));
		// 	// }
		// }
	}

	const int32 numTris = 3 * (MeshSectionData.Indices.Num() / 3);
	for(int32 i = 0; i < numTris; i += 3)
	{
		Builder.AddTriangle(MeshSectionData.Indices[i], MeshSectionData.Indices[i + 1], MeshSectionData.Indices[i + 2]);
	}

    const int32 curLOD = 0;

#ifdef USE_PG

	const bool withCollision = true;

	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(curLOD, FName(TEXT("Section_") + FString::FromInt(m_numSections)));
	const FRealtimeMeshSectionKey PolyGroup0SectionKey = FRealtimeMeshSectionKey::CreateForPolyGroup(GroupKey, 0);
	m_Mesh->CreateSectionGroup(GroupKey, StreamSet);
	// m_Mesh->UpdateSectionConfig(PolyGroup0SectionKey, FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, m_numSections), true);
	m_Mesh->UpdateSectionConfig(PolyGroup0SectionKey, FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Dynamic, m_numSections), withCollision);

#else

	// const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::CreateUnique(curLOD);
	// const FRealtimeMeshSectionKey MeshSectionKey = FRealtimeMeshSectionKey::CreateUnique(GroupKey);
	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::Create(curLOD, m_numSections);
	const FRealtimeMeshSectionKey MeshSectionKey = FRealtimeMeshSectionKey::Create(GroupKey, m_numSections);
	m_Mesh->CreateSectionGroup(GroupKey, StreamSet);
    m_Mesh->CreateSection(MeshSectionKey, FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, m_numSections), FRealtimeMeshStreamRange(0, MeshSectionData.Vertices.Num(), 0, MeshSectionData.Indices.Num()), true);

#endif

}

void UGenericTwinStaticRTMComponent::BuildMeshSection(const SMeshSectionData &MeshSectionData)
{
	FRealtimeMeshStreamSet streamSet;
	RealtimeMesh::TRealtimeMeshBuilderLocal<void, void, void, 1, void> meshBuilder(streamSet);

	if (FRealtimeMeshStream *triangleStream = streamSet.Find(FRealtimeMeshStreams::Triangles))
	{
		if (MeshSectionData.Vertices.Num() > 65535)
		{
			triangleStream->ConvertTo<TIndex3<uint32>>();
		}
		else
		{
			triangleStream->ConvertTo<TIndex3<uint16>>();
		}
	}

	const bool hasNormals = MeshSectionData.Normals != nullptr && MeshSectionData.Normals->Num() == MeshSectionData.Vertices.Num();
    if(hasNormals)
    {
		meshBuilder.EnableTangents(RealtimeMesh::GetRealtimeMeshDataElementType<FPackedNormal>());
    }

	const bool hasUV0 = MeshSectionData.UV0 != nullptr && MeshSectionData.UV0->Num() == MeshSectionData.Vertices.Num();
	const bool hasUV1 = hasUV0 && MeshSectionData.UV1 != nullptr && MeshSectionData.UV1->Num() == MeshSectionData.Vertices.Num();
	TOptional<RealtimeMesh::TRealtimeMeshStridedStreamBuilder<FVector2f, void>> uv1Builder;
	if(hasUV0)
	{
		const int32 numChannels = MeshSectionData.UV1 != nullptr ? 2 : 1;
		const auto texCoordType = RealtimeMesh::GetRealtimeMeshDataElementType<FVector2f>();
		meshBuilder.EnableTexCoords(texCoordType, numChannels);

		if(hasUV1)
		{
			FRealtimeMeshStream *texStream = streamSet.Find(RealtimeMesh::FRealtimeMeshStreams::TexCoords);
			if (texStream)
			{
				uv1Builder = RealtimeMesh::TRealtimeMeshStridedStreamBuilder<FVector2f, void>(*texStream, 1);
			}
		}
	}

	for(int32 i = 0; i < MeshSectionData.Vertices.Num(); ++i)
	{
		const FVector &p = MeshSectionData.Vertices[i];
		auto curVertex = meshBuilder.AddVertex(FVector3f(p.X, p.Y, p.Z));

		if (hasNormals)
		{
			const FVector &n = MeshSectionData.Normals->operator[] (i);
			curVertex.SetNormal(FVector3f(n.X, n.Y, n.Z));
		}

		if (hasUV0)
		{
			const FVector2D &tc0 = MeshSectionData.UV0->operator[] (i);
			curVertex.SetTexCoord(0, FVector2f(tc0.X, tc0.Y));

			if (hasUV1)
			{
				const FVector2D &tc1 = MeshSectionData.UV1->operator[] (i);
				curVertex.SetTexCoord(1, FVector2f(tc1.X, tc1.Y));
			}
		}
	}


	const int32 numTris = 100;// (MeshSectionData.Indices.Num() / 3) * 3;
	for(int32 i = 0; i < numTris; i += 3)
	{
		meshBuilder.AddTriangle(MeshSectionData.Indices[i], MeshSectionData.Indices[i + 1], MeshSectionData.Indices[i + 2]);
	}

	m_Mesh->SetupMaterialSlot(m_numSections, FName(TEXT("MaterialSlot_") + FString::FromInt(m_numSections)));

    const int32 curLOD = 0;
	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::CreateUnique(curLOD);
	const FRealtimeMeshSectionKey MeshSectionKey = FRealtimeMeshSectionKey::CreateUnique(GroupKey);
	m_Mesh->CreateSectionGroup(GroupKey, streamSet);
    m_Mesh->CreateSection(MeshSectionKey, FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, m_numSections), FRealtimeMeshStreamRange(0, MeshSectionData.Vertices.Num(), 0, MeshSectionData.Indices.Num()), true);

}

void UGenericTwinStaticRTMComponent::SetMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
	if	(	m_Mesh != 0
		&&	ElementIndex >= 0
		&&	ElementIndex < m_numSections
		)
	{
		m_Mesh->SetupMaterialSlot(ElementIndex, FName(TEXT("MaterialSlot_") + FString::FromInt(ElementIndex)), Material);
	}
}
