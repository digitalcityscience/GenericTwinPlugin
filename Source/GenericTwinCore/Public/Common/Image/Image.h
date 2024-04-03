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

class GENERICTWINCORE_API SImage
{
public:

	enum class EFormat : uint8
	{
		Unknown,
		GrayScale,
		GrayScaleAlpha,
		RGB,
		RGBA,

	};

	~SImage();

	uint8* Allocate(int32 Width, int32 Height, EFormat Format);

	int32 GetWidth() const;
	int32 GetHeight() const;
	int32 GetNumComponents() const;
	EFormat GetFormat() const;

	uint8* GetData();
	const uint8* GetData() const;
	int32 GetDataSize() const;

	FIntVector2 GetAlphaRange() const;

	void Copy(const SImagePtr &Other, int32 X, int32 Y);

	bool SaveAsBMP(const FString &FileName) const;

	void AsColorArray(TArray<FColor> &DstArray);

private:

	int32						m_Width = 0;
	int32						m_Height = 0;

	EFormat						m_Format = EFormat::Unknown;

	uint8						*m_ImageData = 0;
	int32						m_ImageDataSize = 0;

};

inline int32 SImage::GetWidth() const
{
	return m_Width;
}

inline int32 SImage::GetHeight() const
{
	return m_Height;
}


inline int32 SImage::GetNumComponents() const
{
	switch(m_Format)
	{
		case EFormat::GrayScale:
			return 1;
		case EFormat::GrayScaleAlpha:
			return 2;
		case EFormat::RGB:
			return 3;
		case EFormat::RGBA:
			return 4;
	}
	return 0;
}

inline SImage::EFormat SImage::GetFormat() const
{
	return m_Format;
}

inline uint8* SImage::GetData()
{
	return m_ImageData;
}

inline const uint8* SImage::GetData() const
{
	return m_ImageData;
}

inline int32 SImage::GetDataSize() const
{
	return m_ImageDataSize;
}

}	//	namespace GenericTwin
