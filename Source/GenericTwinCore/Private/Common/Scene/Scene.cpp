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

#include "Common/Scene/Scene.h"
#include "Common/Scene/ISceneNodeVisitor.h"


namespace GenericTwin
{

SScene::SScene()
{
	m_RootNode = new SSceneNode;
	if(m_RootNode)
	{
		m_RootNode->NodeName = TEXT("Root");
	}
}

SScene::~SScene()
{
	delete m_RootNode;
	for(SSceneNode *node : m_AllocatedNodes)
	{
		delete node;
	}

	for(SSceneMesh *mesh : m_Meshes)
	{
		delete mesh;
	}
}

SSceneNode* SScene::CreateSceneNode(SSceneNode *RootNode)
{
	SSceneNode *newNode = new SSceneNode;

	if(newNode)
	{
		m_AllocatedNodes.Add(newNode);

		if(RootNode)
		{
			RootNode->ChildNodes.Add(newNode);
		}
		else if(m_RootNode)
		{
			m_RootNode->ChildNodes.Add(newNode);
		}
	}

	return newNode;
}

SSceneMaterial* SScene::GetOrCreateSceneMaterial(const FString &Name)
{
	if(m_Materials.Contains(Name) == false)
	{
		m_Materials.Add(Name, new SSceneMaterial(Name));
	}

	return m_Materials[Name];
}

SSceneMesh* SScene::CreateMesh(const FString &Name)
{
	SSceneMesh *mesh = new SSceneMesh;
	if(mesh)
	{
		mesh->MeshName = Name;
		mesh->MeshIndex = m_Meshes.Num();
		m_Meshes.Add(mesh);
	}
	return mesh;
}

void SScene::Visit(ISceneNodeVisitor &Visitor)
{
	if(m_RootNode)
	{
		Visitor.BeginVisitScene(*this);

		m_RootNode->Visit(*this, Visitor);

		Visitor.EndVisitScene(*this);
	}
}

uint32 SScene::EstimateGeometryMemorySize() const
{
	uint32 memSize = 0;

	for(const SSceneMesh *mesh : m_Meshes)
	{
		memSize += 12 * static_cast<uint32> (mesh->Vertices.Num());
		memSize += 12 * static_cast<uint32> (mesh->Normals.Num());
		memSize += 8 * static_cast<uint32> (mesh->UV0.Num());
		memSize += 4 * static_cast<uint32> (mesh->Indices.Num());
	}

	return memSize;
}

TArray<FString> SScene::GetMaterialNames() const
{
	TArray<FString> keys;
	m_Materials.GetKeys(keys);
	return keys;
}

void SSceneNode::Visit(const SScene &Scene, ISceneNodeVisitor &Visitor)
{
	if(Visitor.BeginVisitNode(Scene, *this))
	{
		if(Visitor.VisitNode(Scene, *this))
		{
			for(SSceneNode *childNode : ChildNodes)
			{
				childNode->Visit(Scene, Visitor);
			}
		}

		Visitor.EndVisitNode(Scene, *this);
	}
}

}	//	namespace GenericTwin
