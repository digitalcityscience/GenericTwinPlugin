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


#include "Common/BaseTypes.h"

class UMaterialInterface;

namespace GenericTwin
{

class ISceneNodeVisitor;

struct GENERICTWINCORE_API SSceneMaterial
{
	SSceneMaterial()
	{	}

	SSceneMaterial(const FString &_Name)
		:	MaterialName(_Name)
	{	}

	FString							MaterialName;
	TMap<FString, FVector4>			Parameters;

	TMap<FString, FString>			Textures;

	MaterialProxyPtr				MaterialPtr;
	// UMaterialInterface				*Material = 0;
};


struct GENERICTWINCORE_API SSceneMesh
{
	FString						MeshName;
	int32						MeshIndex = -1;

	TArray< FVector >			Vertices;
	TArray< FVector >			Normals;
	TArray< FVector2D >			UV0;

	TArray< int32 >				Indices;

	FString						MaterialName;

};

struct GENERICTWINCORE_API SSceneNode
{
	FString						NodeName;
	FTransform					LocalTransform;
	FTransform					WorldTransform;
	TArray<SSceneNode*>			ChildNodes;

	TArray<SSceneMesh*>			Meshes;

	void Visit(const SScene &Scene, ISceneNodeVisitor &Visitor);

};

class GENERICTWINCORE_API SScene
{
public:

	SScene();

	~SScene();

	SSceneNode* CreateSceneNode(SSceneNode *RootNode);

	SSceneMaterial* GetOrCreateSceneMaterial(const FString &Name);

	TArray<FString> GetMaterialNames() const;
	SSceneMaterial* GetSceneMaterial(const FString& Name);
	const SSceneMaterial* GetSceneMaterial(const FString& Name) const;

	SSceneMesh* CreateMesh(const FString &Name);

	int32 GetMeshCount() const;
	SSceneMesh* GetMesh(int32 MeshIndex) const;

	SSceneNode* GetRootNode() const;

	void Visit(ISceneNodeVisitor &Visitor);

	uint32 EstimateGeometryMemorySize() const;

	/*
	*	Apply material the whole scene
	*/
	void ApplyMaterial(const FString &MaterialReference);
	//void ResolveMaterials(const TMap<FString, GenericTwin::SMaterialMappingItem> &ExactMaterialMap, const FString &MaterialReference);

private:

	SSceneNode							*m_RootNode;
	TArray<SSceneNode*>					m_AllocatedNodes;
	TMap<FString, SSceneMaterial*>		m_Materials;
	TArray<SSceneMesh*>					m_Meshes;

};


inline SSceneMaterial* SScene::GetSceneMaterial(const FString& Name)
{
	return m_Materials.Contains(Name) ? m_Materials[Name] : 0;
}

inline const SSceneMaterial* SScene::GetSceneMaterial(const FString& Name) const
{
	return m_Materials.Contains(Name) ? m_Materials[Name] : 0;
}

inline int32 SScene::GetMeshCount() const
{
	return m_Meshes.Num();
}

inline SSceneMesh* SScene::GetMesh(int32 MeshIndex) const
{
	return MeshIndex < m_Meshes.Num() ? m_Meshes[MeshIndex] : 0;
}

inline SSceneNode* SScene::GetRootNode() const
{
	return m_RootNode;
}


}	//	namespace GenericTwin
