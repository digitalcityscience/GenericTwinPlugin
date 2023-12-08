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

#include "Layers/LayerTypes.h"

#include "UObject/NoExportTypes.h"
#include "TileManager.generated.h" 

DECLARE_LOG_CATEGORY_EXTERN(LogTileManager, Log, All);

namespace GenericTwin {

class ILocationMapper;
class TTileId;
class LayerDependencyBase;

}	//	namespace GenericTwin 


UCLASS()
class UTileManager	:	public UObject
{
	GENERATED_BODY()

public:

	struct SConfiguration
	{
		double					MaxViewDistance = 0.0;
		double					MaxLoadDistance = 0.0;
		int32					ViewRectHalfSize = 0;

		uint32					MaxHeapMemoroyUsage = 0;
		uint32					MaxGPUMemoryUsage = 0;

		int32					PurgeInterval = 0;
	};

	void Configure(const FString &Name, const SConfiguration &Configuration);

	void SetDependency(GenericTwin::LayerDependencyBasePtr Dependency);

	void SetupTiling(GenericTwin::ILocationMapper &LocationMapper, int32 MaxPendingTiles);

	void Shutdown();

	void UpdateView(const FDateTime& CurrentTime, APawn &ViewPawn, const FConvexVolume &ViewFrustum, TArray<GenericTwin::TTileId>& MissingTiles);

	void UpdateTiles(const FDateTime& CurrentTime);

	void AddTile(const GenericTwin::TTileId &TileId, const TileBasePtr &TilePtr);

	TileBasePtr GetTile(const GenericTwin::TTileId &TileId);

	ELayerCoverage GetCoverage(const FBox2d &Area) const;

	void DrawTileBorders();

private:

	void Purge(const FDateTime &CurrentTime);

	void CheckForTilesToLoad();

	void HandlePendingTiles(const FDateTime &CurrentTime);

	TArray<GenericTwin::TTileId> SortVisibleTilesByDistance();


	FString										m_OwnerName;

	GenericTwin::ILocationMapper				*m_LocationMapper = 0;

	int32										m_ViewRectHalfSize = 0;
	double										m_MaxViewDistance = 0.0;
	double										m_MaxLoadDistance = 0.0;

	GenericTwin::LayerDependencyBasePtr			m_Dependency;

	UPROPERTY()
	TArray< UTileBase* >						m_AllocatedTiles;

	TMap<GenericTwin::TTileId, TileBasePtr>		m_Tiles;

	TArray<TileBasePtr>							m_PendingTiles;
	int32										m_MaxPendingTiles = 0;

	FVector										m_CurrentViewPos;
	GenericTwin::TTileId						m_CurrentTileId;
	FIntVector2									m_CurrentTileCoord;

	TSet<GenericTwin::TTileId>					m_VisibleTiles;

	uint32										m_HeapMemoryUsage = 0;
	uint32										m_EstimatedGPUMemoryUsage = 0;

	uint32										m_MaxHeapMemoryUsage = 0;
	uint32										m_MaxGPUMemoryUsage = 0;

	int32										m_PurgeInterval = -1;
	FDateTime									m_LastPurge;

};

