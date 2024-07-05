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

UGenericTwinStaticRTMComponent::~UGenericTwinStaticRTMComponent()
{

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

		if(MeshSection.Vertices.Num() < 65536)
		{
			BuildMeshSection<uint16>(msd);
		}
		else
		{
			BuildMeshSection<uint32>(msd);
		}

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
        SMeshSectionData msd = {Indices, Vertices, Normals, UV0, UV1};

		if(Vertices.Num() < 65536)
		{
			BuildMeshSection<uint16>(msd);
		}
		else
		{
			BuildMeshSection<uint32>(msd);
		}

		const int32 sectionIndex = m_numSections;
		++m_numSections;

		SetMaterial(sectionIndex, Material);

		return sectionIndex;
	}
    return -1;
}

int32 UGenericTwinStaticRTMComponent::AddMeshSections(const TArray<const GenericTwin::SMeshSection*> &MeshSections, const TArray<UMaterialInterface*> &Materials)
{
    return -1;
}

template<typename INDEXTYPE>
void UGenericTwinStaticRTMComponent::BuildMeshSection(const SMeshSectionData &MeshSectionData)
{
	FRealtimeMeshStreamSet StreamSet;
	TRealtimeMeshBuilderLocal<INDEXTYPE, FPackedNormal, FVector2DHalf, 1> Builder(StreamSet);

    if(MeshSectionData.Normals != nullptr)
    {
    	Builder.EnableTangents();
    }

	if(MeshSectionData.UV0 != nullptr)
	{
		Builder.EnableTexCoords();
	}

	for(int32 i = 0; i < MeshSectionData.Vertices.Num(); ++i)
	{
        const FVector& n = MeshSectionData.Normals->operator[] (i);
		auto &vertexBuilder = Builder.AddVertex(FVector3f(MeshSectionData.Vertices[i]))
		.SetNormal(FVector3f(n.X, n.Y, n.Z));
        if(MeshSectionData.UV0 != 0)
		{
			vertexBuilder.SetTexCoord(FVector2f((*MeshSectionData.UV0)[i]));
			if(MeshSectionData.UV1 != 0)
			{
				vertexBuilder.SetTexCoord(1, FVector2f((*MeshSectionData.UV1)[i]));
			}
		}
	}

	const int32 numTris = 3 * (MeshSectionData.Indices.Num() / 3);
	for(int32 i = 0; i < numTris; i += 3)
	{
		Builder.AddTriangle(MeshSectionData.Indices[i], MeshSectionData.Indices[i + 1], MeshSectionData.Indices[i + 2]);
	}

	m_Mesh->SetupMaterialSlot(m_numSections, FName(TEXT("MaterialSlot_") + FString::FromInt(m_numSections)));

    const int32 curLOD = 0;
	const FRealtimeMeshSectionGroupKey GroupKey = FRealtimeMeshSectionGroupKey::CreateUnique(curLOD);
	const FRealtimeMeshSectionKey MeshSectionKey = FRealtimeMeshSectionKey::CreateUnique(GroupKey);
	m_Mesh->CreateSectionGroup(GroupKey, StreamSet);
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
