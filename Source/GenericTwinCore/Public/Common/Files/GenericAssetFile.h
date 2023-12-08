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

class GENERICTWINCORE_API FGenericAssetFile
{
private:

	enum
	{
		EMagicKey = static_cast<uint32> ('G') | (static_cast<uint32> ('T') << 8) | (static_cast<uint32> ('B') << 16) | (static_cast<uint32> ('A') << 24)
	};


	enum class EAssetChunkTypes : uint32
	{
		Texture2D = 1,
		MeshSection,

		CustomData = 0x10000,

	};

	struct FTexture2DChunk	:	public FGTBChunkHeader
	{
		FTexture2DChunk(uint32 TexDataSize)
			:	FGTBChunkHeader(static_cast<uint32> (EAssetChunkTypes::Texture2D), TexDataSize + offsetof(FTexture2DChunk, Data) - sizeof(FGTBChunkHeader))
		{
		}

		uint32		TextureId;

		uint32		Width;
		uint32		Height;
		uint32		Depth;			// when > 0 considered a Texture2DArray with Depth as number of slices, otherwise considered as plain 2D texture
		uint32		Format;

		uint32		DataSize;

		uint8		Data[1];
	};

	struct FMeshSectionChunk	:	public FGTBChunkHeader
	{
		FMeshSectionChunk(uint32 DataSize)
			:	FGTBChunkHeader(static_cast<uint32> (EAssetChunkTypes::MeshSection), DataSize + offsetof(FMeshSectionChunk, DataStream) - sizeof(FGTBChunkHeader))
		{
		}

		uint32		MaterialId = 0;
		uint32		NumVertices = 0;

		uint32		NumNormals = 0;
		uint32		NumUV0 = 0;
		uint32		NumUV1 = 0;
		uint32		NumColors = 0;

		uint32		NumIndices = 0;

		uint8		DataStream[1];

	};

	struct FCustomDataChunk	:	public FGTBChunkHeader
	{
		FCustomDataChunk(uint32 _CustomDataId, uint32 _DataSize)
			:	FGTBChunkHeader(static_cast<uint32> (EAssetChunkTypes::CustomData), _DataSize + offsetof(FCustomDataChunk, Data) - sizeof(FGTBChunkHeader))
			,	CustomDataId(_CustomDataId)
			,	DataSize(_DataSize)
		{
		}

		uint32		CustomDataId;
		uint32		DataSize;
		uint8		Data[1];
	};

public:

	FGenericAssetFile();

	FGenericAssetFile(const void *Data, uint32 NumBytes);

	bool BeginFile(const FString &Url);
	bool BeginCacheFile(const FString &Url, const FString &CacheSlot);

	void BeginTexture2D(uint32 Width, uint32 Height, uint32 Depth, uint32 Format, uint32 TotalSize);
	void AddToTexture2D(const void *Data, uint32 DataSize);

	void AddMeshSection(uint32 MaterialId, const SMeshSection &MeshSection);

	void AddCustomData(uint32 CustomDataId, const void *CustomData, uint32 CustomDataSize);


	bool LoadFromFile(const FString &Url);
	bool LoadFromCache(const FString &Url, const FString &CacheSlot);

	uint32 GetNumTextures2D() const;
	bool IsTexture2DArray(uint32 Index) const;
	const void* GetTexture2D(uint32 Index, uint32& Width, uint32& Height, uint32& Format, uint32& DataSize) const;
	const void* GetTexture2DArray(uint32 Index, uint32& Width, uint32& Height, uint32& Depth, uint32& Format, uint32& DataSize) const;

	uint32 GetNumMeshSections() const;
	void GetMeshSection(uint32 Index, GenericTwin::SMeshSection &Section) const;

	uint32 GetNumCustomDataChunks() const;
	const void*  GetCustomDataChunk(uint32 Index, uint32 *CustomDataId = 0, uint32 *CustomDataSize = 0) const;


private:

	FString								m_Url;
	FCacheKey							m_CacheKey;

	GenericTwin::SCachedFilePtr			m_CachedFile;

	TArray<const FTexture2DChunk*>		m_TextureChunks;
	TArray<const FMeshSectionChunk*>	m_MeshSectionChunks;
	TArray<const FCustomDataChunk*>		m_CustomDataChunks;

};

inline uint32 FGenericAssetFile::GetNumTextures2D() const
{
	return static_cast<uint32> (m_TextureChunks.Num());
}

inline bool FGenericAssetFile::IsTexture2DArray(uint32 Index) const
{
	return m_TextureChunks.Num() ? m_TextureChunks[Index]->Depth > 0 : false;
}


inline uint32 FGenericAssetFile::GetNumMeshSections() const
{
	return static_cast<uint32> (m_MeshSectionChunks.Num());
}

inline uint32 FGenericAssetFile::GetNumCustomDataChunks() const
{
	return static_cast<uint32> (m_CustomDataChunks.Num());
}


}	//	namespace GenericTwin
