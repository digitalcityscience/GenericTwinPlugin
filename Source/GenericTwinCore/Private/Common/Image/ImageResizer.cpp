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

#include "Common/Image/ImageResizer.h"

#include "GenericPlatform/GenericPlatformMath.h"

#include "lancir.h"

namespace GenericTwin {

SImagePtr ImageResizer::Resize(SImagePtr SourceImage, int32 NewSize, bool KeepAspectRatio)
{
	int32 newWidth = 0;
	int32 newHeight = 0;
	if(SourceImage->GetWidth() >= SourceImage->GetHeight())
	{
		newWidth = NewSize;
		if(KeepAspectRatio)
		{
			newHeight = FGenericPlatformMath::Max(1, SourceImage->GetHeight() * NewSize / SourceImage->GetWidth());
		}
		else if(SourceImage->GetHeight() > NewSize)
		{
			newHeight = NewSize;
		}
	}
	else
	{
		newHeight = NewSize;
		if(KeepAspectRatio)
		{
			newWidth = FGenericPlatformMath::Max(1, SourceImage->GetWidth() * NewSize / SourceImage->GetHeight());
		}
		else if(SourceImage->GetHeight() > NewSize)
		{
			newWidth = NewSize;
		}
	}


	SImagePtr resized(new SImage);
	uint8 *imgDataPtr = resized ? resized->Allocate(newWidth, newHeight, SourceImage->GetFormat()) : 0;

	if(imgDataPtr)
	{
		avir::CLancIR lancir;
		lancir.resizeImage<uint8, uint8>(SourceImage->GetData(), SourceImage->GetWidth(), SourceImage->GetHeight(), 0, imgDataPtr, newWidth, newHeight, 0, SourceImage->GetNumComponents());
	}

	return resized;
}

}	//	namespace GenericTwin
