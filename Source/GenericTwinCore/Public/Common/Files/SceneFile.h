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

#include "Common/Files/GTBFileDefinitions.h"

namespace GenericTwin
{

class GENERICTWINCORE_API FSceneFile
{
private:

	enum
	{
		EMagicKey = static_cast<uint32> ('G') | (static_cast<uint32> ('T') << 8) | (static_cast<uint32> ('S') << 16) | (static_cast<uint32> ('F') << 24)
	};


	enum class EAssetChunkTypes : uint32
	{
		SceneNode = 1,
		SceneMesh,
		SceneMaterial,

		Name,

	};

	// struct FSceneMaterial	:	public FGTBChunkHeader
	// {
	// 	FSceneMaterial()
	// 		:	FSceneMaterial(static_cast<uint32> (EAssetChunkTypes::SceneMaterial), offsetof(FSceneMaterial, Data) - sizeof(FGTBChunkHeader))
	// 	{
	// 	}

	// 	uint32					MaterialNameIndex;


	// 	// FString							MaterialName;
	// 	// TMap<FString, FVector4>			Parameters;

	// 	// TMap<FString, FString>			Textures;

	// };


	struct FSceneMeshChunk	:	public FGTBChunkHeader
	{
		FSceneMeshChunk()
			:	FGTBChunkHeader(static_cast<uint32> (EAssetChunkTypes::SceneMesh), offsetof(FSceneMeshChunk, DataStream) - sizeof(FGTBChunkHeader))
		{
		}

		void AddDataSize(uint32 DataSize)
		{
			ChunkSize += DataSize;
		}

		uint32		MaterialNameId = 0;
		uint32		NumVertices = 0;

		uint32		NumNormals = 0;
		uint32		NumUV0 = 0;
		uint32		NumUV1 = 0;
		uint32		NumColors = 0;

		uint32		NumIndices = 0;

		/* 
			variable data stored in data stream in the following order:
			- vertices, normals, uv0, uv1, colors, indices

			The actual number of attributes specified above can be used to determine how many data is written/needs to be read back

		*/ 
		uint8		DataStream[1];

	};

	struct FSceneNodeChunk	:	public FGTBChunkHeader
	{
		FSceneNodeChunk(uint32 DataSize)
			:	FGTBChunkHeader(static_cast<uint32> (EAssetChunkTypes::SceneNode), offsetof(FSceneNodeChunk, DataStream) - sizeof(FGTBChunkHeader))
		{
		}

		uint32		NodeNameId = 0;

		double		LocalPostion[3];
		double		LocalOrientation[4];
		double		LocalScale[3];

		double		WorldPostion[3];
		double		WorldOrientation[4];
		double		WorldScale[3];

		uint32		NumChildNodes = 0;

		uint32		NumMeshes = 0;

		uint8		DataStream[1];		

	};

	struct FNameChunk	:	public FGTBChunkHeader
	{
		FNameChunk(const char *NamePtr, size_t _NameLength)
			:	FGTBChunkHeader(static_cast<uint32> (EAssetChunkTypes::Name), offsetof(FNameChunk, Name) - sizeof(FGTBChunkHeader))
		{
			NameLength = _NameLength;
			ChunkSize += (_NameLength + 3) & 0xFFFFFFFC;
		}

		uint32		NameLength = 0;
		char		Name[1];
	};


public:

	FSceneFile();

	FSceneFile(const void *Data, uint32 NumBytes);

	bool SaveScene(const ScenePtr&Scene, const FString &Url);
	bool SaveSceneToCache(const ScenePtr&Scene, const FString &Url, const FString &CacheSlot);

	ScenePtr LoadFromFile(const FString &Url);
	ScenePtr LoadFromCache(const FString &Url, const FString &CacheSlot);

private:

	void SaveSceneMeshesToCache(const ScenePtr &Scene, const FCacheKey &CacheKey);
	void SaveSceneNodesToCache(const ScenePtr &Scene, const FCacheKey &CacheKey);
	void SaveNames(const FCacheKey& CacheKey);

	uint32 AddName(const FString &Name);

	FString								m_Url;

	// GenericTwin::SCachedFilePtr			m_CachedFile;

	TMap<FString, uint32>				m_NameCache;
	TArray<FString>						m_Names;

	TArray<FSceneNodeChunk>				m_NodeChunks;
	TArray<FSceneMeshChunk>				m_MeshChunks;
	// TArray<FSceneMaterial>				m_MaterialChunks;
	TArray<FNameChunk>					m_NameChunks;


};


}	//	namespace GenericTwin
