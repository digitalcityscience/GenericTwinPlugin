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

#include "Layers/TileLayerBase.h"
#include "SpatialPartitioning/TileManager/TilingHelpers.h"


UTileLayerBase::UTileLayerBase()
{
}

void UTileLayerBase::OnEndPlay(FTwinWorldContext& TwinWorldCtx)
{
	if(m_TileManager)
		m_TileManager->Shutdown();
}

void UTileLayerBase::OnVisibilityChanged(FTwinWorldContext &TwinWorldCtx)
{
	m_TileManager->SetVisibility(m_isVisible);
}

void UTileLayerBase::UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds)
{
	if	(	TwinWorldCtx.Pawn != nullptr
		&&	TwinWorldCtx.SceneView != nullptr
		&&	m_TileManager
		)
	{
		const FDateTime curTime = FDateTime::Now();

		TArray<GenericTwin::TTileId> missingTiles;
		m_TileManager->UpdateView(curTime, *TwinWorldCtx.Pawn, TwinWorldCtx.SceneView->CullingFrustum, missingTiles);

		const int32_t maxTilesToCreate = 5;
		for(int32 i = 0; i < missingTiles.Num() && i < maxTilesToCreate; ++i)
		{
			const GenericTwin::TTileId &tileId = missingTiles[i];
			TileBasePtr newTile = CreateTile(TwinWorldCtx, tileId);
			m_TileManager->AddTile(tileId, newTile);
		}

		m_TileManager->UpdateTiles(curTime, DeltaSeconds);

		if(m_DrawTileBorders)
		{
			m_TileManager->DrawTileBorders();
		}
	}
}

void UTileLayerBase::ConfigureTileManager(const FString &Name, const UTileManager::SConfiguration &Configuration)
{
	if(CreateTileManager())
		m_TileManager->Configure(Name, Configuration);
}

void UTileLayerBase::SetupTiling(GenericTwin::ILocationMapper &LocationMapper, int32 MaxPendingTiles)
{
	if(CreateTileManager())
		m_TileManager->SetupTiling(LocationMapper, MaxPendingTiles);
}

void UTileLayerBase::ClearTiles()
{
	if(m_TileManager)
		m_TileManager->ClearTiles();
}

bool UTileLayerBase::CreateTileManager()
{
	if(m_TileManager == 0)
	{
		m_TileManager = NewObject<UTileManager>(GetWorld(), FName(), RF_Standalone);
	}
	return m_TileManager != 0;
}

TileBasePtr UTileLayerBase::CreateTile(FTwinWorldContext &TwinWorldCtx, const GenericTwin::TTileId& TileId)
{
	return TileBasePtr();
}

TileBasePtr UTileLayerBase::GetTile(const GenericTwin::TTileId &TileId)
{
	return m_TileManager->GetTile(TileId);
}

void UTileLayerBase::SetDependency(GenericTwin::LayerDependencyBasePtr Dependency)
{
	if(m_TileManager)
		m_TileManager->SetDependency(Dependency);
}

ELayerCoverage UTileLayerBase::GetCoverage(const FBox2d &Area) const
{
	return m_TileManager ? m_TileManager->GetCoverage(Area) : ELayerCoverage::Undefined;
}

void UTileLayerBase::ForEachTile( FProcessTileFunction ProcessTileFunc )
{
	m_TileManager->ForEachTile(ProcessTileFunc);
}
