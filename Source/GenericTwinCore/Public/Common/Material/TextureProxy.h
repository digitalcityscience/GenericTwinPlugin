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

#include "CoreMinimal.h"

#include "Common/Material/GenericTwinTextureCache.h"

namespace GenericTwin {

struct GENERICTWINCORE_API STextureProxy
{
public:

	STextureProxy();

	STextureProxy(uint32 _TextureId, UTexture2D *_Texture, uint32 _TextureMemSize);

	STextureProxy(uint32 _TextureId, UTexture2DArray *_TextureArray, uint32 _TextureMemSize);

	~STextureProxy();

	bool IsValid() const;

	UTexture2D* GetTexture2D() const;
	UTexture2DArray* GetTexture2DArray() const;

	uint32 GetTextureMemorySize() const;

private:

	uint32					TextureId = 0;

	UTexture2D				*Texture2D = 0;
	UTexture2DArray			*Texture2DArray = 0;

	uint32					TextureMemSize = 0;
};


inline STextureProxy::STextureProxy()
{
}

inline STextureProxy::STextureProxy(uint32 _TextureId, UTexture2D *_Texture, uint32 _TextureMemSize)
	:	TextureId(_TextureId)
	,	Texture2D(_Texture)
	,	TextureMemSize(_TextureMemSize)
{
}

inline STextureProxy::STextureProxy(uint32 _TextureId, UTexture2DArray *_TextureArray, uint32 _TextureMemSize)
	:	TextureId(_TextureId)
	,	Texture2DArray(_TextureArray)
	,	TextureMemSize(_TextureMemSize)
{
}

inline STextureProxy::~STextureProxy()
{
	UGenericTwinTextureCache::GetInstance().ReleaseTexture(TextureId);
}

inline bool STextureProxy::IsValid() const
{
	return TextureId > 0 && (Texture2DArray || Texture2D);
}

inline UTexture2D* STextureProxy::GetTexture2D() const
{
	return Texture2D;
}

inline UTexture2DArray* STextureProxy::GetTexture2DArray() const
{
	return Texture2DArray;
}

inline uint32 STextureProxy::GetTextureMemorySize() const
{
	return TextureMemSize;
}

}	//	namespace GenericTwin
