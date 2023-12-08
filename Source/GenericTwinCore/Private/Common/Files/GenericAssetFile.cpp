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

#include "Common/Files/GenericAssetFile.h"
#include "Common/Caching/LocalFileCache.h"

namespace GenericTwin
{

FGenericAssetFile::FGenericAssetFile()
	:	m_CacheKey()
{
}

FGenericAssetFile::FGenericAssetFile(const void *Data, uint32 NumBytes)
{
}

bool FGenericAssetFile::BeginFile(const FString &Url)
{
	m_Url = Url;
	m_CacheKey = FCacheKey();

	return false;
}

bool FGenericAssetFile::BeginCacheFile(const FString &Url, const FString &CacheSlot)
{
	m_Url = Url;
	m_CacheKey = FCacheKey(Url);
	m_CacheKey.SetSlot(CacheSlot);

	LocalFileCache &lfc = LocalFileCache::GetInstance();

	FGTBFileHeader fileHeader(EMagicKey);
	bool res = lfc.BeginFile(m_CacheKey, &fileHeader, sizeof(fileHeader));

	if(res)
	{

	}
	else
	{
		m_CacheKey = FCacheKey();
	}

	return res;
}

void FGenericAssetFile::BeginTexture2D(uint32 Width, uint32 Height, uint32 Depth, uint32 Format, uint32 TotalSize)
{
	if(m_CacheKey)
	{
		FTexture2DChunk texChunk(TotalSize);

		texChunk.TextureId;

		texChunk.Width = Width;
		texChunk.Height = Height;
		texChunk.Depth = Depth;
		texChunk.Format = Format;
		texChunk.DataSize = TotalSize;
		
		LocalFileCache::GetInstance().AddToFile(m_CacheKey, &texChunk, offsetof(FTexture2DChunk, Data));
	}

}

void FGenericAssetFile::AddToTexture2D(const void *Data, uint32 DataSize)
{
	if (m_CacheKey)
	{
		LocalFileCache::GetInstance().AddToFile(m_CacheKey, Data, DataSize);
	}
}

void FGenericAssetFile::AddMeshSection(uint32 MaterialId, const SMeshSection &MeshSection)
{
	const uint32 dataSize =		static_cast<uint32>
								(	MeshSection.Vertices.Num() * sizeof(FVector)
								+	MeshSection.Normals.Num() * sizeof(FVector)
								+	MeshSection.UV0.Num() * sizeof(FVector2D)
								+	MeshSection.UV1.Num() * sizeof(FVector2D)
								+	MeshSection.Colors.Num() * sizeof(FColor)
								+	MeshSection.Indices.Num() * sizeof(int32)
								);

	FMeshSectionChunk chunk(dataSize);

	chunk.MaterialId = MaterialId;
	chunk.NumVertices = static_cast<uint32> (MeshSection.Vertices.Num());
	chunk.NumNormals = static_cast<uint32> (MeshSection.Normals.Num());
	chunk.NumUV0 = static_cast<uint32> (MeshSection.UV0.Num());
	chunk.NumUV1 = static_cast<uint32> (MeshSection.UV1.Num());
	chunk.NumColors = static_cast<uint32> (MeshSection.Colors.Num());
	chunk.NumIndices = static_cast<uint32> (MeshSection.Indices.Num());


	if (m_CacheKey)
	{
		LocalFileCache &lfc = LocalFileCache::GetInstance();
		lfc.AddToFile(m_CacheKey, &chunk, offsetof(FMeshSectionChunk, DataStream));
		lfc.AddToFile(m_CacheKey, MeshSection.Vertices.GetData(), MeshSection.Vertices.Num() * sizeof(FVector));
		lfc.AddToFile(m_CacheKey, MeshSection.Normals.GetData(), MeshSection.Normals.Num() * sizeof(FVector));
		lfc.AddToFile(m_CacheKey, MeshSection.UV0.GetData(), MeshSection.UV0.Num() * sizeof(FVector2D));
		lfc.AddToFile(m_CacheKey, MeshSection.UV1.GetData(), MeshSection.UV1.Num() * sizeof(FVector2D));
		lfc.AddToFile(m_CacheKey, MeshSection.Colors.GetData(), MeshSection.Colors.Num() * sizeof(FColor));
		lfc.AddToFile(m_CacheKey, MeshSection.Indices.GetData(), MeshSection.Indices.Num() * sizeof(int32));
	}
}

void FGenericAssetFile::AddCustomData(uint32 CustomDataId, const void *CustomData, uint32 CustomDataSize)
{
	if (m_CacheKey)
	{
		FCustomDataChunk chunk(CustomDataId, CustomDataSize);

		LocalFileCache &lfc = LocalFileCache::GetInstance();
		lfc.AddToFile(m_CacheKey, &chunk, offsetof(FCustomDataChunk, Data));
		lfc.AddToFile(m_CacheKey, CustomData, CustomDataSize);
	}
}

bool FGenericAssetFile::LoadFromCache(const FString &Url, const FString &CacheSlot)
{
	bool loaded = false;

	GenericTwin::FCacheKey key(Url);
	key.SetSlot(CacheSlot);

	m_CachedFile = GenericTwin::LocalFileCache::GetInstance().GetCachedFile(key);
	if	(	m_CachedFile
		&&	m_CachedFile->IsValid()
		&&	m_CachedFile->DataSize >= sizeof(FGTBFileHeader)
		)
	{
		const FGTBFileHeader *fileHeader = static_cast<const FGTBFileHeader*>(m_CachedFile->Data);
		if(fileHeader->MagicKey == EMagicKey)
		{
			const uint8* curPtr = reinterpret_cast<const uint8*> (fileHeader + 1);
			int32 bytesLeft = m_CachedFile->DataSize - sizeof(FGTBFileHeader);

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
						case EAssetChunkTypes::Texture2D:
							m_TextureChunks.Add(reinterpret_cast<const FTexture2DChunk*> (chunkHeader));
							break;

						case EAssetChunkTypes::MeshSection:
							m_MeshSectionChunks.Add(reinterpret_cast<const FMeshSectionChunk*> (chunkHeader));
							break;

						case EAssetChunkTypes::CustomData:
							m_CustomDataChunks.Add(reinterpret_cast<const FCustomDataChunk*> (chunkHeader));
							break;
					}
				}
			}

			loaded = bytesLeft == 0;
		}
		
	}

	return loaded;
}


const void* FGenericAssetFile::GetTexture2DArray(uint32 Index, uint32& Width, uint32& Height, uint32& Depth, uint32& Format, uint32& DataSize) const
{
	if	(	static_cast<int32> (Index) < m_TextureChunks.Num()
		&&	m_TextureChunks[Index]->Depth > 0
		)
	{
		Width = m_TextureChunks[Index]->Width;
		Height = m_TextureChunks[Index]->Height;
		Depth = m_TextureChunks[Index]->Depth;
		Format = m_TextureChunks[Index]->Format;
		DataSize = m_TextureChunks[Index]->DataSize;

		return &(m_TextureChunks[Index]->Data[0]);
	}

	return 0;
}

void FGenericAssetFile::GetMeshSection(uint32 Index, GenericTwin::SMeshSection &Section) const
{
	if(static_cast<int32> (Index) < m_MeshSectionChunks.Num())
	{
		const FMeshSectionChunk *chunk = m_MeshSectionChunks[Index];
		const uint8 *ptr = &chunk->DataStream[0];
		if(chunk->NumVertices)
		{
			Section.Vertices = TArray<FVector>(reinterpret_cast<const FVector*>(ptr), chunk->NumVertices);
			ptr += chunk->NumVertices * sizeof(FVector);
		}

		if(chunk->NumNormals)
		{
			Section.Normals = TArray<FVector>(reinterpret_cast<const FVector*>(ptr) , chunk->NumNormals);
			ptr += chunk->NumNormals * sizeof(FVector);
		}
		if(chunk->NumUV0)
		{
			Section.UV0 = TArray<FVector2D>(reinterpret_cast<const FVector2D*>(ptr) , chunk->NumUV0);
			ptr += chunk->NumUV0 * sizeof(FVector2D);
		}
		if(chunk->NumUV1)
		{
			Section.UV1 = TArray<FVector2D>(reinterpret_cast<const FVector2D*>(ptr) , chunk->NumUV1);
			ptr += chunk->NumUV1 * sizeof(FVector2D);
		}
		if(chunk->NumColors)
		{
			Section.Colors = TArray<FLinearColor>(reinterpret_cast<const FLinearColor*>(ptr) , chunk->NumColors);
			ptr += chunk->NumColors * sizeof(FLinearColor);
		}
		if(chunk->NumIndices)
		{
			Section.Indices = TArray<int32>(reinterpret_cast<const int32*>(ptr) , chunk->NumIndices);
			ptr += chunk->NumIndices * sizeof(int32);
		}
	}
}

const void* FGenericAssetFile::GetCustomDataChunk(uint32 Index, uint32 *CustomDataId, uint32 *CustomDataSize) const
{
	if(static_cast<int32> (Index) < m_CustomDataChunks.Num())
	{
		const FCustomDataChunk *chunk = m_CustomDataChunks[Index];

		if(CustomDataId)
			*CustomDataId = chunk->CustomDataId;

		if(CustomDataSize)
			*CustomDataSize = chunk->DataSize;

		return &chunk->Data[0];
	}

	return 0;
}

}	//	namespace GenericTwin
