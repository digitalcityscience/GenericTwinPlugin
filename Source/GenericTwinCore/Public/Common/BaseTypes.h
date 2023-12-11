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

#include <type_traits>

namespace GenericTwin
{

struct SMeshSection
{
	TArray< FVector >			Vertices;
	TArray< FVector >			Normals;
	TArray< FVector2D >			UV0;
	TArray< FVector2D >			UV1;
	TArray< FLinearColor >		Colors;
	TArray< int32 >				Indices;

	UMaterialInterface			*Material = 0;

	void Clear()
	{
		Vertices.Empty();
		Normals.Empty();
		UV0.Empty();
		Colors.Empty();
		Indices.Empty();
	}

	bool IsEmpty() const
	{
		return Vertices.IsEmpty() || Indices.IsEmpty();
	}

};
typedef TSharedPtr<SMeshSection>	MeshSectionPtr;

class SScene;
typedef TSharedPtr<SScene>	ScenePtr;
typedef TSharedPtr<const SScene>	SceneConstPtr;

class SImage;
typedef TSharedPtr<SImage> SImagePtr;

struct STextureProxy;
typedef TSharedPtr<STextureProxy> TextureProxyPtr;

struct SMaterialProxy;
typedef TSharedPtr<SMaterialProxy> MaterialProxyPtr;

class FGenericAssetFile;
typedef TSharedPtr<FGenericAssetFile> GenericAssetFilePtr;

class FCacheKey;

template<typename E>
constexpr auto CastToType(E e)
{
	return static_cast<typename std::underlying_type<E>::type>(e);
}


}	//	namespace GenericTwin

