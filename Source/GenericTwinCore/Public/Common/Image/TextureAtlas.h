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


namespace GenericTwin {


class GENERICTWINCORE_API FTextureAtlas
{
private:

	enum class ESplitMode : uint8
	{
		None,
		Exact,
		Vertical,
		Horizontal,
		RotatedVertical,
		RotatedHorizontal,
	};

	struct FNode
	{
		FNode(uint32 _Width = 0, uint32 _Height = 0)
			:	X(0)
			,	Y(0)
			,	Width(_Width)
			,	Height(_Height)
			,	Area(_Width * _Height)
		{
		}

		bool FitsExactly(uint32 ImageWidth, uint32 ImageHeight) const
		{
			return Width == ImageWidth && Height == ImageHeight;
		}

		void UpdateArea()
		{
			Area = Width * Height;
		}

		ESplitMode GetSplit(uint32 ImageWidth, uint32 ImageHeight, uint32 ImageArea, uint32 &remainingArea) const;

		uint32				X;
		uint32				Y;
		uint32				Width;
		uint32				Height;

		uint32				Area;
	};

	struct FTextureNode
	{
		FNode			Node;
		bool			IsRotated;
	};

public:

	FTextureAtlas(uint32 AtlasSize, uint32 Margin);

	~FTextureAtlas();

	bool AddTexture(const FString &TextureUrl, const SImagePtr &ImagePtr);

	FVector2D GetTextureCoordinate(const FString &TextureUrl, const FVector2D &TexCoord);

	void Compress(bool WithAlphaChannel);

	EPixelFormat GetPixelFormat() const;

	const void* GetTextureData() const;

	int32 GetTextureDataSize() const;

	const SImagePtr& GetAtlasImage() const;

	int32 GetRemainingArea() const;

private:

	int32 SplitNode(int32 NodeIndex, ESplitMode SplitMode, uint32 ImageWidth, uint32 ImageHeight);

	void InsertTexture(int32 NodeIndex, const FString &TextureUrl, const SImagePtr &Image, bool IsRotated);

	int32 AllocateNode(uint32 Width = 0, uint32 Height = 0);


	TArray<FNode>					m_Nodes;
	TMap<FString, FTextureNode>		m_Textures;

	TSet<int32>						m_FreeNodes;

	uint32							m_TextureAtlasSize = 0;

	uint32							m_Margin = 0;

	uint32							m_AvailablePixels = 0;

	EPixelFormat					m_PixelFormat = PF_R8G8B8A8;

	SImagePtr						m_AtlasImage;

	const uint8*					m_CompressedImage = 0;
	int32							m_CompressedImageSize = 0;

};

inline EPixelFormat FTextureAtlas::GetPixelFormat() const
{
	return m_PixelFormat;
}

inline const void* FTextureAtlas::GetTextureData() const
{
	return m_PixelFormat == EPixelFormat::PF_DXT5 || m_PixelFormat == EPixelFormat::PF_DXT1 ? m_CompressedImage : m_AtlasImage->GetData();
}

inline int32 FTextureAtlas::GetTextureDataSize() const
{
	return m_PixelFormat == EPixelFormat::PF_DXT5 || m_PixelFormat == EPixelFormat::PF_DXT1 ? m_CompressedImageSize : m_AtlasImage->GetDataSize();
}

inline const SImagePtr& FTextureAtlas::GetAtlasImage() const
{
	return m_AtlasImage;
}

inline int32 FTextureAtlas::GetRemainingArea() const
{
	return m_AvailablePixels;
}

}	//	namespace GenericTwin
