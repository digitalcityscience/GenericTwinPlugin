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

/*

Idee:
Das konkrete Tile-Klasse stellt Mapping von WordLocation -> TileId (Tiling schema and tile size in meters) zur Verfügung

UTileManager
- bekommt FunctionPtr um World Location auf TileId zu mappen
- bekommt TileSize (für Distanceberechnung)
- ReferencePosition bekommt er nicht

Tile
- bringt mapping Function und hat somit volle Kontrolle über Mapping vonm TileId auf data blob (i.e. CityGML file), welches z.B. in eigenem
  ReferenceSystem abgewlegt ist, siehe Kachelunbg LGV oder Helsinki


*/

#include "SpatialPartitioning/TileManager/TileManager.h"
#include "SpatialPartitioning/TileManager/ILocationMapper.h"

#include "Layers/LayerDependencyBase.h"

#include "GeoReferencingSystem.h"

#include "ConvexVolume.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogTileManager);

void UTileManager::Configure(const FString &Name, const SConfiguration &Configuration)
{
	m_OwnerName = Name;

	m_MaxViewDistance = Configuration.MaxViewDistance;
	m_MaxLoadDistance = Configuration.MaxLoadDistance;
	m_ViewRectHalfSize = Configuration.ViewRectHalfSize;
	m_MaxHeapMemoryUsage = Configuration.MaxHeapMemoroyUsage;
	m_MaxGPUMemoryUsage = Configuration.MaxGPUMemoryUsage;
	m_PurgeInterval = Configuration.PurgeInterval;
}

void UTileManager::SetDependency(GenericTwin::LayerDependencyBasePtr Dependency)
{
	m_Dependency = Dependency;
}

void UTileManager::SetupTiling(GenericTwin::ILocationMapper &LocationMapper, int32 MaxPendingTiles)
{
	m_LocationMapper = &LocationMapper;
	m_MaxPendingTiles = MaxPendingTiles;
}

void UTileManager::Shutdown()
{
	for(UTileBase *tile : m_AllocatedTiles)
	{
		if(tile)
			tile->Shutdown();
	}
}

void UTileManager::SetVisibility(bool NewVisibility)
{
	m_IsVisible = NewVisibility;
}

void UTileManager::UpdateView(const FDateTime& CurrentTime, APawn &ViewPawn, const FConvexVolume &ViewFrustum, TArray<GenericTwin::TTileId> &MissingTiles)
{
	if (m_LocationMapper)
	{
		m_CurrentViewPos = ViewPawn.GetActorLocation();

		m_CurrentTileId = m_LocationMapper->MapLocationToTileId(m_CurrentViewPos);
		m_CurrentTileCoord = m_CurrentTileId.Get();

		// determine, which tiles are currently visible
		TSet<GenericTwin::TTileId> currentlyVisible;
		TArray< TPair<GenericTwin::TTileId, uint32> > missing;
		for (int32 y = m_CurrentTileCoord.Y - m_ViewRectHalfSize; y < (m_CurrentTileCoord.Y + m_ViewRectHalfSize); ++y)
		{
			for (int32 x = m_CurrentTileCoord.X - m_ViewRectHalfSize; x < (m_CurrentTileCoord.X + m_ViewRectHalfSize); ++x)
			{
				const GenericTwin::TTileId curTileId(x, y);
				//if (curTileId != m_CurrentTileId) continue;
				if (m_Tiles.Contains(curTileId))
				{
					TileBasePtr curTile = m_Tiles[curTileId];
					if (curTile)
					{
						const FVector& curTileCtr = curTile->GetTileCenter();
						const FVector& curTileExtent = curTile->GetTileExtent();
						const double curDistance = (curTileCtr - m_CurrentViewPos).Size();

						if (curDistance < m_MaxViewDistance
							&& (ViewFrustum.Planes.IsEmpty() || ViewFrustum.IntersectBox(curTileCtr, curTileExtent))
							)
						{
							if(m_IsVisible)
							{
								currentlyVisible.Add(curTileId);
								curTile->UpdateTileVisibility(1.0f);
							}

							curTile->Touch(CurrentTime);
						}
					}
				}
				else
				{
					if	(	m_Dependency == 0
						||	m_Dependency->IsDependencyMet(m_LocationMapper->GetTileArea(curTileId))
						)
					{
						FVector curTileCtr = m_LocationMapper->GetTileCenter(curTileId);

						const double distance = (curTileCtr - m_CurrentViewPos).Size();
						if (distance <= m_MaxLoadDistance)
						{
							missing.Add({ curTileId, static_cast<uint32_t> (distance * 0.01) });
						}
					}
				}
			}
		}

		if(missing.Num() > 1)
			missing.Sort([](const TPair<GenericTwin::TTileId, uint32> &a, const TPair<GenericTwin::TTileId, uint32> &b) { return a.Value < b.Value; });
		
		for(const auto &p : missing)
			MissingTiles.Add(p.Key);

		// remove currently visible tiles from set of tiles previously visible
		for (const auto& tileId : currentlyVisible)
		{
			m_VisibleTiles.Remove(tileId);
		}

		// update visibility of tiles not visible anymore
		for (const auto& tileId : m_VisibleTiles)
		{
			if (m_Tiles.Contains(tileId))
			{
				TileBasePtr curTile = m_Tiles[tileId];
				if (curTile)
				{
					curTile->UpdateTileVisibility(0.0f);
				}
			}
		}

		// preserve currently tiles
		m_VisibleTiles = currentlyVisible;
	}
}

void UTileManager::DrawTileBorders()
{
	if(GetWorld())
	{
		for(const auto &item : m_Tiles)
		{
			const TileBasePtr &tile = item.Value;
			if(tile)
			{
				const FVector& curTileCtr = tile->GetTileCenter();
				const FVector& curTileExtent = tile->GetTileExtent();

				FColor color(220, 40, 80, 255);

				const float tileVis = tile->GetTileVisibility();
				if(tileVis == 1.0)
				{
					color = FColor(20, 240, 80, 255);
				}

				DrawDebugBox(GetWorld(), curTileCtr, curTileExtent, color, false, 0.0f, 64, 100.0f);
			}
		}
	}
}

void UTileManager::ForEachTile( FProcessTileFunction ProcessTileFunc )
{
	for(const auto &item : m_Tiles)
	{
		const TileBasePtr &tile = item.Value;
		if	(	tile
			&&	tile->GetTileLoadState() == ETileLoadState::Loaded
			)
		{
			ProcessTileFunc.Execute( *(tile.Get()) );
		}
	}
}

void UTileManager::UpdateTiles(const FDateTime &CurrentTime)
{
	HandlePendingTiles(CurrentTime);

	CheckForTilesToLoad();

	if(m_PurgeInterval > 0)
	{
		const FTimespan elapsed = CurrentTime - m_LastPurge;
		if(elapsed.GetTotalSeconds() > m_PurgeInterval)
		{
			Purge(CurrentTime);
			m_LastPurge = CurrentTime;
		}
	}
}

void UTileManager::HandlePendingTiles(const FDateTime &CurrentTime)
{
	TArray<int32> tilesToRemove;
	for(int32 i = 0; i < m_PendingTiles.Num(); ++i)
	{
		TileBasePtr &tilePtr = m_PendingTiles[i];
		if(tilePtr->GetTileLoadState() == ETileLoadState::Loaded)
		{
			tilesToRemove.Add(i);
			uint32 tileHeapUsage = 0;
			uint32 tileGPUUsage = 0;

			tilePtr->GetMemoryUsage(tileHeapUsage, tileGPUUsage);
			tilePtr->Touch(CurrentTime);

			m_HeapMemoryUsage += tileHeapUsage;
			m_EstimatedGPUMemoryUsage += tileGPUUsage;
		}
	}

	for(int32 i = tilesToRemove.Num() - 1; i >= 0; --i)
	{
		m_PendingTiles.RemoveAt(tilesToRemove[i]);
	}
}

void UTileManager::CheckForTilesToLoad()
{
	int32 remainingPending = m_MaxPendingTiles - m_PendingTiles.Num();
	if(remainingPending > 0)
	{
		struct STile
		{
			STile(const TileBasePtr &_Tile, uint32 _Distance, bool _IsVisible)
				:	Tile(_Tile)
				,	Distance(_Distance)
				,	IsVisible(_IsVisible)
			{	}

			TileBasePtr		Tile;
			int32			Distance;
			bool			IsVisible;

			bool operator < (const STile &Other) const
			{
				if(IsVisible == Other.IsVisible)
					return Distance < Other.Distance;

				const int32 d0 = IsVisible ? -Distance : Distance;
				const int32 d1 = Other.IsVisible ? -Other.Distance : Other.Distance;
				return d0 < d1;
			}
		};

		TArray<STile> potentialTiles;
		// first collect all visible tiles with unknown load state
		for(const GenericTwin::TTileId &tileId : m_VisibleTiles)
		{
			if (m_Tiles.Contains(tileId))
			{
				TileBasePtr curTile = m_Tiles[tileId];
				if	(	curTile
					&&	curTile->GetTileLoadState() == ETileLoadState::Unknown
					)
				{
					const FVector& curTileCtr = curTile->GetTileCenter();
					const FVector& curTileExtent = curTile->GetTileExtent();
					const double curDistance = (curTileCtr - m_CurrentViewPos).Size();
					potentialTiles.Add(STile(curTile, static_cast<int32_t> (curDistance * 0.01), true));
				}
			}
		}

		if(potentialTiles.Num() < remainingPending)
		{
			for(auto &item : m_Tiles)
			{
				TileBasePtr &curTile = item.Value;
				if	(	curTile
					&&	curTile->GetTileLoadState() == ETileLoadState::Unknown	
					)
				{
					const FVector& curTileCtr = curTile->GetTileCenter();
					const FVector& curTileExtent = curTile->GetTileExtent();
					const double curDistance = (curTileCtr - m_CurrentViewPos).Size();
					potentialTiles.Add(STile(curTile, static_cast<int32_t> (curDistance * 0.01), false));
				}
			}
		}

		// Only sort, when there are more potential tiles than we can handle at once
		if(potentialTiles.Num() > remainingPending)
			potentialTiles.Sort();

		for(STile &curTile : potentialTiles)
		{
			UE_LOG(LogTileManager, Log, TEXT("[%s] Adding %s to pending tiles"), *m_OwnerName, *(curTile.Tile->GetTileId().ToString()));
			
			m_PendingTiles.Add(curTile.Tile);
			curTile.Tile->Load();

			--remainingPending;
			if (remainingPending <= 0)
				break;
		}
	}
}

void UTileManager::AddTile(const GenericTwin::TTileId &TileId, const TileBasePtr &TilePtr)
{
	if (m_Tiles.Contains(TileId))
	{
		m_Tiles[TileId] = TilePtr;
	}
	else
	{
		m_Tiles.Add(TileId, TilePtr);
		m_AllocatedTiles.Add(TilePtr.Get());
	}
}

TileBasePtr UTileManager::GetTile(const GenericTwin::TTileId &TileId)
{
	return m_Tiles.Contains(TileId) ? m_Tiles[TileId] : TileBasePtr();
}

ELayerCoverage UTileManager::GetCoverage(const FBox2d &Area) const
{
	ELayerCoverage coverage = ELayerCoverage::NotCovered;

	if(m_LocationMapper)
	{
		GenericTwin::TTileId minTileId = m_LocationMapper->MapLocationToTileId(FVector(Area.Min, 0.0));
		FIntVector2	minTileCoord = minTileId.Get();

		GenericTwin::TTileId maxTileId = m_LocationMapper->MapLocationToTileId(FVector(Area.Max, 0.0));
		FIntVector2	maxTileCoord = maxTileId.Get();

		int32 numCovered = 0;
		int32 numLoaded = 0;
		int32 numTotal = 0;
		for(int32 y = maxTileCoord.Y; y <= minTileCoord.Y; ++y)
		{
			for(int32 x = minTileCoord.X; x <= maxTileCoord.X; ++x)
			{
				++numTotal;
				GenericTwin::TTileId curTileId(x, y);
				if(m_Tiles.Contains(curTileId))
				{
					TileBasePtr tilePtr = m_Tiles[curTileId];
					if(tilePtr)
					{
						++numCovered;
						if(tilePtr->GetTileLoadState() == ETileLoadState::Loaded)
							++numLoaded;
					}
				}
			}
		}

		if(numLoaded == numTotal)
		// {
			coverage = ELayerCoverage::FullyCovered;
		// 	UE_LOG(LogTileManager, Log, TEXT("Tile fully covered for %s <-> %s"), *(Area.Min.ToString()), *(Area.Max.ToString()));
		// }
		else if(numLoaded > 0)
			coverage = ELayerCoverage::PartlyCovered;
		else if(numCovered > 0)
			coverage = ELayerCoverage::PotentionallyCovered;
	}

	return coverage;
}

void UTileManager::Purge(const FDateTime &CurrentTime)
{
	struct STile
	{
		bool operator <(const STile &Other) const
		{
			return	Distance < Other.Distance;
		}

		UTileBase			*Tile;
		double				Distance = 0;				//	distance in meters
		int32				LastTouched = 0;			//	last touched in seconds
		uint32				GPUMemoryUsage = 0;
	};

	const int32 MinLifeTime = 10;
	
	int32 exceededGPUMem = m_EstimatedGPUMemoryUsage - m_MaxGPUMemoryUsage;

	if	(	m_HeapMemoryUsage > m_MaxHeapMemoryUsage
		||	exceededGPUMem > 0
		)
	{
		const int32 MB = 1024 * 1024;
		UE_LOG(LogTileManager, Log, TEXT("{[%s] Purge required %d/%d Heap %d/%d GPU"), *m_OwnerName, m_HeapMemoryUsage / MB, m_MaxHeapMemoryUsage / MB, m_EstimatedGPUMemoryUsage / MB, m_MaxGPUMemoryUsage / MB);

		TArray<STile> tiles;
		for(UTileBase *curTile : m_AllocatedTiles)
		{
			if	(	curTile
				&&	m_VisibleTiles.Contains(curTile->GetTileId()) == false
				)
			{
				const FTimespan elapsed = CurrentTime - curTile->GetLastTouched();
				const int32 lastTouched = static_cast<int32> (elapsed.GetTotalSeconds());
				const ETileLoadState tileLoadState = curTile->GetTileLoadState();

				switch(tileLoadState)
				{
					case ETileLoadState::Unknown:
						break;

					case ETileLoadState::Loading:
					case ETileLoadState::Unloading:
						break;

					case ETileLoadState::Loaded:
						if(lastTouched > MinLifeTime)
						{
							uint32 tileHeapUsage = 0;
							uint32 tileGPUUsage = 0;
							curTile->GetMemoryUsage(tileHeapUsage, tileGPUUsage);
							if(tileGPUUsage > 0)
							{
								const FVector& curTileCtr = curTile->GetTileCenter();
								const FVector& curTileExtent = curTile->GetTileExtent();
								const double curDistance = (curTileCtr - m_CurrentViewPos).Size();

								if(curDistance > m_MaxLoadDistance)
								{
									tiles.Add( {curTile, curDistance, lastTouched, tileGPUUsage} );
								}
							}
						}
						break;

					case ETileLoadState::Empty:
						break;
				}
			}
		}

		UE_LOG(LogTileManager, Log, TEXT("Identified %d tiles for purging."), tiles.Num());

		if(tiles.Num() > 1)
		{
			tiles.Sort();
		}

		for(int32 i = tiles.Num() - 1; i >= 0 && exceededGPUMem > 0; --i)
		{
			const GenericTwin::TTileId tileId = tiles[i].Tile->GetTileId();

			UE_LOG(LogTileManager, Log, TEXT("Purging tile %s %s at distance %d"), *(tileId.ToString()), *(tiles[i].Tile->GetTileName()), static_cast<int32>(tiles[i].Distance));

			UTileBase *tile = tiles[i].Tile;

			if(tile)
			{
				tile->Unload();
				m_AllocatedTiles.Remove(tile);
			}

			m_Tiles.Remove(tileId);

			m_EstimatedGPUMemoryUsage -= tiles[i].GPUMemoryUsage;
			exceededGPUMem -= tiles[i].GPUMemoryUsage;
		}

		UE_LOG(LogTileManager, Log, TEXT("{[%s] Purging done %d/%d Heap %d/%d GPU"), *m_OwnerName, m_HeapMemoryUsage / MB, m_MaxHeapMemoryUsage / MB, m_EstimatedGPUMemoryUsage / MB, m_MaxGPUMemoryUsage / MB);

	}
}
