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

#include "Common/Files/SceneFile.h"
#include "Common/Caching/LocalFileCache.h"
#include "Common/Scene/Scene.h"
#include "Common/Scene/ISceneNodeVisitor.h"

namespace GenericTwin
{

FSceneFile::FSceneFile()
{
}

FSceneFile::FSceneFile(const void *Data, uint32 NumBytes)
{
}

bool FSceneFile::SaveScene(const ScenePtr &Scene, const FString &Url)
{
	return false;
}

bool FSceneFile::SaveSceneToCache(const ScenePtr &Scene, const FString &Url, const FString &CacheSlot)
{
	FCacheKey cacheKey(Url);
	cacheKey.SetSlot(CacheSlot);

	LocalFileCache &lfc = LocalFileCache::GetInstance();

	FGTBFileHeader fileHeader(EMagicKey);
	bool res = lfc.BeginFile(cacheKey, &fileHeader, sizeof(fileHeader));

	if(res)
	{
		SaveSceneMeshesToCache(Scene, cacheKey);
		SaveSceneNodesToCache(Scene, cacheKey);

		SaveNames(cacheKey);
	}

	return res;
}

void FSceneFile::SaveSceneMeshesToCache(const ScenePtr &Scene, const FCacheKey &CacheKey)
{
	LocalFileCache &lfc = LocalFileCache::GetInstance();
	const int32 numSceneMeshes = Scene->GetMeshCount();
	for(signed i = 0; i < numSceneMeshes; ++i)
	{
		const SSceneMesh *sceneMesh = Scene->GetMesh(i);

		FSceneMeshChunk meshChunk;
		meshChunk.MaterialNameId = AddName(sceneMesh->MaterialName);
		meshChunk.NumVertices = static_cast<uint32> (sceneMesh->Vertices.Num());
		meshChunk.NumNormals = static_cast<uint32> (sceneMesh->Normals.Num());
		meshChunk.NumUV0 = static_cast<uint32> (sceneMesh->UV0.Num());
		meshChunk.NumUV1 = 0;
		meshChunk.NumColors = 0;
		meshChunk.NumIndices = static_cast<uint32> (sceneMesh->Indices.Num());
		meshChunk.AddDataSize(sceneMesh->Vertices.Num() * sizeof(FVector) + sceneMesh->Normals.Num() * sizeof(FVector) + sceneMesh->UV0.Num() * sizeof(FVector2D) + sceneMesh->Indices.Num() * sizeof(int32));
		lfc.AddToFile(CacheKey, &meshChunk, offsetof(FSceneMeshChunk, DataStream));

		lfc.AddToFile(CacheKey, sceneMesh->Vertices.GetData(), sceneMesh->Vertices.Num() * sizeof(FVector));
		lfc.AddToFile(CacheKey, sceneMesh->Normals.GetData(), sceneMesh->Normals.Num() * sizeof(FVector));
		lfc.AddToFile(CacheKey, sceneMesh->UV0.GetData(), sceneMesh->UV0.Num() * sizeof(FVector2D));
		// lfc.AddToFile(CacheKey, sceneMesh->UV1.GetData(), sceneMesh->UV1.Num() * sizeof(FVector2D));
		// lfc.AddToFile(CacheKey, sceneMesh->Colors.GetData(), sceneMesh->Colors.Num() * sizeof(FColor));
		lfc.AddToFile(CacheKey, sceneMesh->Indices.GetData(), sceneMesh->Indices.Num() * sizeof(int32));
	}

}

void FSceneFile::SaveSceneNodesToCache(const ScenePtr &Scene, const FCacheKey &CacheKey)
{
	struct SNode
	{
		SNode(const SSceneNode *_SceneNode)
			:	SceneNode(_SceneNode)
		{
		}

		const SSceneNode		*SceneNode = 0;
		TArray<int32>			ChildNodes;
	};

	class SaveNodesVisitor	:	public ISceneNodeVisitor
	{
	public:

		virtual ~SaveNodesVisitor()
		{
		}

		virtual bool BeginVisitNode(const SScene &Scene, SSceneNode &Node)
		{
			int32 nodeIndex = m_Nodes.Num();
			m_Nodes.Add( SNode(&Node) );
			if(m_ParentNodes.IsEmpty() == false)
			{
				m_Nodes[m_ParentNodes.Last()].ChildNodes.Add(nodeIndex);
			}

			m_ParentNodes.Add(nodeIndex);

			return true;
		}

		virtual bool VisitNode(const SScene &Scene, SSceneNode &Node)
		{
			return true;
		}

		virtual void EndVisitNode(const SScene &Scene, SSceneNode &Node)
		{
			const int32 index = m_ParentNodes.Num() - 1;
			if(index >= 0)
				m_ParentNodes.RemoveAt(index);
		}

		const TArray<SNode> GetNodes() const
		{
			return m_Nodes;
		}

	private:

		TArray<int32>		m_ParentNodes;

		TArray<SNode>		m_Nodes;

	};

	SaveNodesVisitor snv;
	Scene->Visit(snv);

	LocalFileCache& lfc = LocalFileCache::GetInstance();

	for (const SNode& node : snv.GetNodes())
	{
		const SSceneNode *sceneNode = node.SceneNode;
		FSceneNodeChunk sceneNodeChunk(node.ChildNodes.Num() * sizeof(uint32) + sceneNode->Meshes.Num() * sizeof(uint32));

		sceneNodeChunk.NodeNameId = AddName(sceneNode->NodeName);

		FVector v = sceneNode->LocalTransform.GetLocation();
		double *dst = sceneNodeChunk.LocalPostion;
		dst[0] = v.X;	dst[0] = v.Y;	dst[2] = v.Z;

		FQuat q = sceneNode->LocalTransform.GetRotation();
		dst = sceneNodeChunk.LocalOrientation;
		dst[0] = q.W;	dst[1] = q.X;	dst[2] = q.Y;	dst[3] = q.Z;

		v = sceneNode->LocalTransform.GetScale3D();
		dst = sceneNodeChunk.LocalScale;
		dst[0] = v.X;	dst[1] = v.Y;	dst[2] = v.Z;

		v = sceneNode->LocalTransform.GetLocation();
		dst = sceneNodeChunk.WorldPostion;
		dst[0] = v.X;	dst[0] = v.Y;	dst[2] = v.Z;

		q = sceneNode->LocalTransform.GetRotation();
		dst = sceneNodeChunk.WorldOrientation;
		dst[0] = q.W;	dst[1] = q.X;	dst[2] = q.Y;	dst[3] = q.Z;

		v = sceneNode->LocalTransform.GetScale3D();
		dst = sceneNodeChunk.WorldScale;
		dst[0] = v.X;	dst[1] = v.Y;	dst[2] = v.Z;

		sceneNodeChunk.NumChildNodes = static_cast<uint32>(sceneNode->ChildNodes.Num());
		sceneNodeChunk.NumMeshes = static_cast<uint32>(sceneNode->Meshes.Num());
		lfc.AddToFile(CacheKey, &sceneNodeChunk, offsetof(FSceneNodeChunk, DataStream));

		if(sceneNodeChunk.NumChildNodes)
			lfc.AddToFile(CacheKey, &(node.ChildNodes[0]), node.ChildNodes.Num() * sizeof(int32));

		if(sceneNodeChunk.NumMeshes)
		{
			TArray<int32> meshIndices;
			for(unsigned i = 0; i < sceneNodeChunk.NumMeshes; ++i)
			{
				const SSceneMesh *sceneMesh = Scene->GetMesh(i);
				meshIndices.Add(sceneMesh->MeshIndex);
			}
			lfc.AddToFile(CacheKey, &(meshIndices[0]), sceneNodeChunk.NumMeshes * sizeof(int32));
		}

	}
}

void FSceneFile::SaveNames(const FCacheKey& CacheKey)
{
	LocalFileCache& lfc = LocalFileCache::GetInstance();
	for(const FString &name : m_Names)
	{
		const char *namePtr = TCHAR_TO_ANSI(*name);
		const size_t nameLength = strlen(namePtr);

		FNameChunk nameChunk(namePtr, nameLength);
		lfc.AddToFile(CacheKey, &nameChunk, offsetof(FNameChunk, Name));

		lfc.AddToFile(CacheKey, namePtr, nameLength);
	}
}

ScenePtr FSceneFile::LoadFromFile(const FString &Url)
{
	return ScenePtr();
}


ScenePtr FSceneFile::LoadFromCache(const FString &Url, const FString &CacheSlot)
{
	GenericTwin::FCacheKey key(Url);
	key.SetSlot(CacheSlot);

	GenericTwin::SCachedFilePtr cachedFile = GenericTwin::LocalFileCache::GetInstance().GetCachedFile(key);
	if	(	cachedFile
		&&	cachedFile->IsValid()
		&&	cachedFile->DataSize >= sizeof(FGTBFileHeader)
		)
	{
		const FGTBFileHeader *fileHeader = static_cast<const FGTBFileHeader*>(cachedFile->Data);
		if(fileHeader->MagicKey == EMagicKey)
		{
			const uint8* curPtr = reinterpret_cast<const uint8*> (fileHeader + 1);
			int32 bytesLeft = cachedFile->DataSize - sizeof(FGTBFileHeader);

			while(bytesLeft >= sizeof(FGTBChunkHeader))
			{
				bytesLeft -= sizeof(FGTBChunkHeader);

				const FGTBChunkHeader *chunkHeader = reinterpret_cast<const FGTBChunkHeader*> (curPtr);
				curPtr += sizeof(FGTBChunkHeader) + chunkHeader->ChunkSize;

				bytesLeft -= chunkHeader->ChunkSize;

				if(bytesLeft >= 0)
				{
					switch(chunkHeader->ChunkId)
					{
						case CastToType(EAssetChunkTypes::SceneNode):
						// 	m_TextureChunks.Add(reinterpret_cast<const FTexture2DChunk*> (chunkHeader));
							break;

						case CastToType(EAssetChunkTypes::SceneMesh):
						// 	m_MeshSectionChunks.Add(reinterpret_cast<const FMeshSectionChunk*> (chunkHeader));
							break;

						case CastToType(EAssetChunkTypes::SceneMaterial):
						// 	m_CustomDataChunks.Add(reinterpret_cast<const FCustomDataChunk*> (chunkHeader));
							break;

						case CastToType(EAssetChunkTypes::Name):
						// 	m_CustomDataChunks.Add(reinterpret_cast<const FCustomDataChunk*> (chunkHeader));
							break;
					}
				}
			}

		}
	}

	return ScenePtr();
}

uint32 FSceneFile::AddName(const FString &Name)
{
	if(m_NameCache.Contains(Name) == false)
	{
		const int32 idx = m_Names.Num();
		m_Names.Add(Name);
		m_NameCache.Add(Name, static_cast<uint32> (idx));
	}

	return m_NameCache[Name];
}


}	//	namespace GenericTwin
