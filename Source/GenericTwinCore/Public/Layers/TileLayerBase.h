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


#include "Layers/LayerBase.h"
#include "SpatialPartitioning/TileManager/TileManager.h"

#include "TileLayerBase.generated.h" 

class UTileManager;
class UTileBase;

UCLASS()
class GENERICTWINCORE_API UTileLayerBase	:	public ULayerBase
{
	GENERATED_BODY()

public:

	UTileLayerBase();

	virtual void OnEndPlay(FTwinWorldContext& TwinWorldCtx) override;

	void ConfigureTileManager(const FString &Name, const UTileManager::SConfiguration &Configuration);
	void SetupTiling(GenericTwin::ILocationMapper &LocationMapper, int32 MaxPendingTiles);
	void ClearTiles();

	virtual void UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds) override;

	virtual void OnVisibilityChanged(FTwinWorldContext &TwinWorldCtx) override;

	virtual TileBasePtr CreateTile(FTwinWorldContext &TwinWorldCtx, const GenericTwin::TTileId &TileId);

	TileBasePtr GetTile(const GenericTwin::TTileId &TileId);

	void SetDependency(GenericTwin::LayerDependencyBasePtr Dependency);

	ELayerCoverage GetCoverage(const FBox2d &Area) const;

	void ForEachTile( FProcessTileFunction ProcessTileFunc );

private:

	bool CreateTileManager();

	UPROPERTY()
	TObjectPtr<UTileManager>		m_TileManager;

protected:

	bool							m_DrawTileBorders = false;

};
