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

#include "Common/Scene/Visitors/FlattenSceneVisitor.h"

#include "Math/NumericLimits.h"


namespace GenericTwin
{

FlattenSceneVisitor::FlattenSceneVisitor(const FTransform &Transform)
	:	m_Transform(Transform)
{
}

FlattenSceneVisitor::~FlattenSceneVisitor()
{
}

void FlattenSceneVisitor::BeginVisitScene(const SScene &Scene)
{
	m_FlattenedScene = GenericTwin::ScenePtr(new SScene);


	if(m_FlattenedScene)
	{
		SSceneNode *rootNode = m_FlattenedScene->GetRootNode();
		if(rootNode)
		{
			TArray<FString> materialNames = Scene.GetMaterialNames();
			for(const auto &matName : materialNames)
			{
				const SSceneMaterial *srcMat = Scene.GetSceneMaterial(matName);
				SSceneMaterial *newMat = m_FlattenedScene->GetOrCreateSceneMaterial(matName);

				if(newMat && srcMat)
				{
					*newMat = *srcMat;

					SSceneMesh *mesh = m_FlattenedScene->CreateMesh(matName + TEXT("_Mesh"));
					if(mesh)
					{
						mesh->MaterialName = matName;
						m_Meshes.Add(matName, mesh);
						rootNode->Meshes.Add(mesh);
					}
				}
			}
		}
	}
}

bool FlattenSceneVisitor::VisitNode(const SScene &Scene, SSceneNode &Node)
{
	for(SSceneMesh *srcMesh : Node.Meshes)
	{
		SSceneMesh *dstMesh = m_Meshes.Contains(srcMesh->MaterialName) ? m_Meshes[srcMesh->MaterialName] : 0;
		if(dstMesh)
		{
			const int32 startIndex = dstMesh->Vertices.Num();

			for(int32 i = 0; i < srcMesh->Vertices.Num(); ++i)
			{
				const FVector &pos = srcMesh->Vertices[i];
				FVector transformed = Node.WorldTransform.TransformPosition(pos);
				transformed += m_Transform.GetTranslation();
				transformed = m_Transform.GetRotation().RotateVector(transformed);
				transformed *= m_Transform.GetScale3D();
				dstMesh->Vertices.Add(transformed);

				if(i < srcMesh->Normals.Num())
				{
					const FVector& nrm = srcMesh->Normals[i];
					FVector transformedNrm = m_Transform.GetRotation().RotateVector(Node.WorldTransform.TransformVector(nrm));
					dstMesh->Normals.Add(transformedNrm);
				}
				else
				{
					dstMesh->Normals.Add(FVector::ZeroVector);
				}

				if(i < srcMesh->UV0.Num())
				{
					dstMesh->UV0.Add(srcMesh->UV0[i]);
				}
				else
				{
					dstMesh->UV0.Add(FVector2D::ZeroVector);
				}
			}

			for(int32 index : srcMesh->Indices)
				dstMesh->Indices.Add(index + startIndex);

		}
	}

	return true;
}


}	//	namespace GenericTwin

