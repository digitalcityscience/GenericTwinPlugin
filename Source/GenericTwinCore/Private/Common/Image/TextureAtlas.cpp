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

#include "Common/Image/TextureAtlas.h"
#include "Common/Image/Image.h"

#include "Common/Image/TextureCompression.h"


namespace GenericTwin {

FTextureAtlas::FTextureAtlas(uint32 AtlasSize, uint32 Margin)
	:	m_TextureAtlasSize(AtlasSize)
	,	m_Margin(Margin)
	,	m_AvailablePixels(AtlasSize * AtlasSize)
{
	//	allocate root node
	AllocateNode(AtlasSize, AtlasSize);
	m_FreeNodes.Add(0);
	m_AtlasImage = SImagePtr(new SImage);
	if(m_AtlasImage)
		m_AtlasImage->Allocate(AtlasSize, AtlasSize, SImage::EFormat::RGBA);
}

FTextureAtlas::~FTextureAtlas()
{
	if(m_CompressedImage)
		delete [] m_CompressedImage;
}

bool FTextureAtlas::AddTexture(const FString &TextureUrl, const SImagePtr &ImagePtr)
{
	bool added = false;

	const uint32 imgWidth = static_cast<uint32> (ImagePtr->GetWidth());
	const uint32 imgHeight = static_cast<uint32> (ImagePtr->GetHeight());
	const uint32 imgArea = imgWidth * imgHeight;

	if(imgArea < m_AvailablePixels)
	{
		int32 bestNodeIndex = -1;
		ESplitMode splitMode = ESplitMode::Horizontal;

		// find best free node
		uint32 minRemainingArea = 0xFFFFFFFF;
		for(int32 nodeIdx : m_FreeNodes)
		{
			const FNode &node = m_Nodes[nodeIdx];
			if(node.Area >= imgArea)
			{
				uint32 curRemainingArea = 0;
				const ESplitMode curSplitMode = node.GetSplit(imgWidth, imgHeight, imgArea, curRemainingArea);
				if(curSplitMode == ESplitMode::Exact)
				{
					splitMode = curSplitMode;
					bestNodeIndex = nodeIdx;
					break;
				}
				else if(curSplitMode != ESplitMode::None)
				{
					if(curRemainingArea < minRemainingArea)
					{
						minRemainingArea = curRemainingArea;
						splitMode = curSplitMode;
						bestNodeIndex = nodeIdx;
					}
				}
			}
		}

		// if rotated mode, switch imgWidth and imgHeight and set splitMode to vert. or hor., set flag isRotated
		bool isRotated = false;

		if(bestNodeIndex >= 0 && splitMode != ESplitMode::None)
		{
			if(splitMode == ESplitMode::Exact)
			{
				InsertTexture(bestNodeIndex, TextureUrl, ImagePtr, isRotated);
				m_FreeNodes.Remove(bestNodeIndex);
				added = true;
			}
			else
			{
				const int32 splitIndex = SplitNode(bestNodeIndex, splitMode, imgWidth, imgHeight);
				if(splitIndex)
				{
					if(m_Nodes[splitIndex].FitsExactly(imgWidth, imgHeight))
					{
						InsertTexture(splitIndex, TextureUrl, ImagePtr, isRotated);
						added = true;
					}
					else
					{
						int32 finalSplitIndex = SplitNode(splitIndex, splitMode == ESplitMode::Vertical ? ESplitMode::Horizontal : ESplitMode::Vertical, imgWidth, imgHeight);
						if(finalSplitIndex >= 0)
						{
							InsertTexture(finalSplitIndex, TextureUrl, ImagePtr, isRotated);
							m_FreeNodes.Add(splitIndex);
							added = true;
						}
					}
				}
			}
		}

		if(added)
			m_AvailablePixels -= imgArea;
	}

	return added;
}

FVector2D FTextureAtlas::GetTextureCoordinate(const FString &TextureUrl, const FVector2D &TexCoord)
{
	FVector2D atlasTexCoord;

	if(m_Textures.Contains(TextureUrl))
	{
		const FTextureNode &texNode = m_Textures[TextureUrl];

		atlasTexCoord.X = static_cast<double> (texNode.Node.X) / static_cast<double> (m_TextureAtlasSize);
		atlasTexCoord.Y = static_cast<double> (texNode.Node.Y) / static_cast<double> (m_TextureAtlasSize);

		atlasTexCoord.X += TexCoord.X * static_cast<double> (texNode.Node.Width) / static_cast<double> (m_TextureAtlasSize);
		atlasTexCoord.Y += (1.0 - TexCoord.Y) * static_cast<double> (texNode.Node.Height) / static_cast<double> (m_TextureAtlasSize);
	}

	return atlasTexCoord;
}

void FTextureAtlas::Compress(bool WithAlphaChannel)
{
	if(WithAlphaChannel)
	{
		m_CompressedImage = GenericTwin::TextureCompression::CompressDXT5(m_AtlasImage, m_CompressedImageSize);
		m_PixelFormat = EPixelFormat::PF_DXT5;
	}
	else
	{
		m_CompressedImage = GenericTwin::TextureCompression::CompressDXT1(m_AtlasImage, m_CompressedImageSize);
		m_PixelFormat = EPixelFormat::PF_DXT1;
	}



}

int32 FTextureAtlas::SplitNode(int32 NodeIndex, ESplitMode SplitMode, uint32 ImageWidth, uint32 ImageHeight)
{
	int32 newNodeIndex = AllocateNode();

	if(newNodeIndex > 0)
	{
		FNode &oldNode = m_Nodes[NodeIndex];
		FNode &newNode = m_Nodes[newNodeIndex];

		newNode = oldNode;

		switch(SplitMode)
		{
			case ESplitMode::Horizontal:
				newNode.Y = oldNode.Y + oldNode.Height - ImageHeight;
				newNode.Height = ImageHeight;
				oldNode.Height -= ImageHeight;					
				break;

			case ESplitMode::Vertical:
				newNode.Width = ImageWidth;
				oldNode.X += ImageWidth;
				oldNode.Width -= ImageWidth;
				break;

		}

		oldNode.UpdateArea();
		newNode.UpdateArea();

		return newNodeIndex;
	}

	return -1;
}

void FTextureAtlas::InsertTexture(int32 NodeIndex, const FString &TextureUrl, const SImagePtr &Image, bool IsRotated)
{
	FNode &node = m_Nodes[NodeIndex];

	FTextureNode texNode;
	texNode.Node = node;
	texNode.IsRotated = IsRotated;

	m_Textures.Add(TextureUrl, texNode);

	m_AtlasImage->Copy(Image, node.X, node.Y);
}

int32 FTextureAtlas::AllocateNode(uint32 Width, uint32 Height)
{
	int32 index = m_Nodes.Num();

	m_Nodes.Add(FNode(Width, Height));


	return index;
}

FTextureAtlas::ESplitMode FTextureAtlas::FNode::GetSplit(uint32 ImageWidth, uint32 ImageHeight, uint32 ImageArea, uint32 &remainingArea) const
{
	ESplitMode splitMode = ESplitMode::None;

	if(FitsExactly(ImageWidth, ImageHeight))
	{
		splitMode = ESplitMode::Exact;
		remainingArea = 0;
	}
	else if(ImageWidth <= Width && ImageHeight <= Height)
	{
		splitMode = ImageWidth >= ImageHeight ? ESplitMode::Horizontal : ESplitMode::Vertical;
		remainingArea = Area - ImageArea;
	}

	return splitMode;
}

}	//	namespace GenericTwin
