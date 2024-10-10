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


class UMaterialInterface;
class USceneComponent;

namespace GenericTwin
{

struct SMeshSection;

class GENERICTWINCORE_API IStaticMeshComponent
{

public:

	enum class EUploadMode
	{
		Immediately,
		GameThread,
		Background
	};

	static IStaticMeshComponent* Create(AActor *Actor);

	virtual ~IStaticMeshComponent();

	virtual void Construct(AActor *Actor, EUploadMode UploadMode) = 0;
	virtual void Initialize(EUploadMode UploadMode) = 0;
	virtual void Clear() = 0;
	virtual int32 AddMeshSection(const SMeshSection &MeshSection, UMaterialInterface *Material) = 0;
	virtual int32 AddMeshSection(const TArray<int32> &Indices, const TArray<FVector> &Vertices, const TArray<FVector> *Normals, const TArray<FVector2D> *UV0, const TArray<FVector2D> *UV1, const TArray<FLinearColor> *Colors, UMaterialInterface *Material) = 0;
	virtual int32 AddMeshSections(const TArray<const SMeshSection*> &MeshSections, const TArray<UMaterialInterface*> &Materials)  = 0;
	virtual USceneComponent* GetSceneComponent() = 0;

	virtual EUploadMode GetUploadMode() const = 0;

};


}	//	namespace GenericTwin
