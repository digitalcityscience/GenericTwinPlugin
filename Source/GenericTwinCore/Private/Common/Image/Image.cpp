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

#include "Common/Image/Image.h"

namespace GenericTwin {
	
SImage::~SImage()
{
	delete [] m_ImageData;
}

uint8* SImage::Allocate(int32 Width, int32 Height, EFormat Format)
{
	if(m_ImageData)
	{
		delete [] m_ImageData;
		m_ImageData = 0;
		m_ImageDataSize = 0;
	}

	m_Width = Width;
	m_Height = Height;
	m_Format = Format;

	m_ImageDataSize = Width * Height * GetNumComponents();
	uint8 *ptr = new uint8[m_ImageDataSize];
	m_ImageData = ptr;

	return ptr;
}

void SImage::Copy(const SImagePtr &Other, int32 X, int32 Y)
{
	if	(	Other->m_Format == m_Format
		&&	X < m_Width
		&&	Y < m_Height
		)
	{
		const int32 width = X + Other->m_Width <= m_Width ? Other->m_Width : m_Width - X;
		const int32 height = Y + Other->m_Height <= m_Height ? Other->m_Height : m_Height - Y;

		const int32 srcStride = (Other->m_Width - width) * GetNumComponents();
		const int32 dstStride = (m_Width - width) * GetNumComponents();

		const uint8 *src = Other->m_ImageData;
		uint8 *dst = m_ImageData + (Y * m_Width + X) * GetNumComponents();

		for(int32 row = 0; row < height; ++row)
		{
			for(int32 col = 0; col < width; ++col)
			{
				for(int32 c = GetNumComponents(); c > 0; --c)
					*dst++ = *src++;
			}
			src += srcStride;
			dst += dstStride;
		}
	}
}

FIntVector2 SImage::GetAlphaRange() const
{
	FIntVector2 range(255, 255);
	if(m_ImageData)
	{
		switch(m_Format)
		{
			case EFormat::GrayScaleAlpha:
				break;

			case EFormat::RGBA:
				{
					range = FIntVector2(255, 0);
					const uint8 *ptr = m_ImageData + 3;
					for(signed i = 0; i < m_ImageDataSize / 4; ++i)
					{
						if(*ptr < range.X)
							range.X = *ptr;
						if(*ptr > range.Y)
							range.Y = *ptr;
						
						ptr += 4;
					}
				}
				break;
		}
	}

	return range;
}

#if WITH_EDITOR

bool SImage::SaveAsBMP(const FString &FileName) const
{
	struct SBmpFileMagic
	{
		unsigned char magic[2];
	};

	struct SBmpFileHeader
	{
		int filesz;
		short creator1;
		short creator2;
		int bmp_offset;
	};

	struct SBitmapInfoHeader
	{
		int header_sz;
		int width;
		int height;
		short nplanes;
		short bitspp;
		int compress_type;
		int bmp_bytesz;
		int hres;
		int vres;
		int ncolors;
		int nimpcolors;
	};

	if(m_Format != EFormat::RGB && m_Format != EFormat::RGBA)
		return false;

	bool res = false;

	const int32 bpp = GetNumComponents();
	const int32 fileSize = m_Width * m_Height * bpp;

	FILE *out = fopen(TCHAR_TO_ANSI(*FileName), "wb");
	if (out)
	{
		SBmpFileMagic bm = { {'B', 'M'} };
		SBmpFileHeader bh = { 54 + fileSize, 0, 0, 54 };
		SBitmapInfoHeader bmpInfoHeader = { 40, m_Width, m_Height, 1, 24, 0, 0, 0, 0, 0, 0 };

		fwrite(&bm, 1, sizeof(bm), out);
		fwrite(&bh, 1, sizeof(bh), out);
		fwrite(&bmpInfoHeader, 1, sizeof(bmpInfoHeader), out);

		const uint32 numPadding = (4 - (m_Width * 3) % 4) % 4;
		char bmpPadBytes[3] = { 0, 0, 0 };
		for (signed i = m_Height - 1; i >= 0; --i)
		{
			int32 lineOffset = m_Width * i * bpp;
			for(int32 p = 0; p < m_Width; ++p)
			{
				fputc(*(m_ImageData + (lineOffset + 2)), out );
				fputc(*(m_ImageData + (lineOffset + 1)), out );
				fputc(*(m_ImageData + (lineOffset)), out );
				lineOffset += bpp;
			}

			if (numPadding)
				fwrite(bmpPadBytes, 1, numPadding, out);
		}

		fclose(out);
		res = true;
	}

	return res;
}

#endif

}	//	namespace GenericTwin
