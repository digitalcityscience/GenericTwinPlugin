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

#include "SpatialPartitioning/TileManager/TileBase.h"

#include "StreamingModelTile.generated.h" 

DECLARE_LOG_CATEGORY_EXTERN(LogStreamingModelTile, Log, All);

class AGenericTwinCustomMeshActor;
class UStreamingModelLayer;

UCLASS()
class GENERICTWINCOMMONLAYERS_API UStreamingModelTile	:	public UTileBase
{
	GENERATED_BODY()

public:

	UStreamingModelTile();

	void SetLayer(UStreamingModelLayer *Layer);

	virtual ~UStreamingModelTile();

	virtual void Shutdown() override;

	virtual bool Load() override;
	virtual void Unload() override;

	virtual void SetIsHidden(bool IsHidden) override;

	virtual void OnTileVisibilityChanged() override;

	virtual FString GetTileName() const override;

	void AddToWorld(const FVector &Location, const GenericTwin::ScenePtr &ScenePtr);

private:

	UStreamingModelLayer						*m_Layer = 0;

	UPROPERTY()
	TObjectPtr<AGenericTwinCustomMeshActor>		m_Actor;

	FString										m_Url;

};


inline void UStreamingModelTile::SetLayer(UStreamingModelLayer *Layer)
{
	m_Layer = Layer;
}
