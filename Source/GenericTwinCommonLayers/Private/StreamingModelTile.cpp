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


#include "StreamingModelTile.h"
#include "Actors/GenericTwinStaticMeshActor.h"

#include "StreamingModelLayer.h"

DEFINE_LOG_CATEGORY(LogStreamingModelTile);

UStreamingModelTile::UStreamingModelTile()
	:	m_Layer(0)
	,	m_Actor(0)
{	
}

UStreamingModelTile::~UStreamingModelTile()
{
	if(m_Actor)
	{
		m_Actor->Destroy();
	}
}

void UStreamingModelTile::Shutdown()
{
	if(m_Actor)
		m_Actor->Release();
}

bool UStreamingModelTile::Load()
{
	if	(	m_Layer
		)
	{
		m_Url = m_Layer->LoadTileModel(m_TileId);
		m_TileLoadState = ETileLoadState::Loading;

		UE_LOG(LogStreamingModelTile, Log, TEXT("StreamingModelTile %s Loading Url %s"), *(m_TileId.ToString()), *m_Url);

	}
	else
	{
		// if nothing got loaded, consider this tile an empty tile
		m_TileLoadState = ETileLoadState::Empty;
	}


	return m_TileLoadState == ETileLoadState::Loading;
}

void UStreamingModelTile::Unload()
{
	if(m_Actor)
	{
		m_Actor->Release();
		m_Actor->Destroy();
		m_Actor = 0;
	}
}

void UStreamingModelTile::SetIsHidden(bool IsHidden)
{
	if(m_Actor)
	{
		m_Actor->SetActorHiddenInGame(IsHidden);
	}
}

void UStreamingModelTile::OnTileVisibilityChanged()
{
	if(m_Actor)
	{
		m_Actor->SetActorHiddenInGame( m_TileVisibility < 1.0 );
	}
}

FString UStreamingModelTile::GetTileName() const
{
	return		m_Actor
			?	FString(TEXT("StreamingModelTile_")) + m_Actor->GetName()
			:	FString(TEXT("StreamingModelTile_"));
}

void UStreamingModelTile::AddToWorld(const FVector &Location, const GenericTwin::ScenePtr &ScenePtr)
{
	// if(m_Actor)
	// {
	// 	m_Actor->SetActorLocation(Location);
	// }
	// else
	// {
	// 	m_Actor = GetWorld()->SpawnActor<AGenericTwinStaticMeshActor>(Location, FRotator::ZeroRotator, FActorSpawnParameters());
	// 	OnTileVisibilityChanged();
	// }

	// if(m_Actor)
	// {
	// 	m_Actor->SetFlattenedScene(ScenePtr);

	// 	UE_LOG(LogStreamingModelTile, Log, TEXT("StreamingModelTile %s[%s] added for %s <-> %s Url: %s"), *(GetTileName()), *(m_TileId.ToString()), *((GetTileCenter() - GetTileExtent()).ToString()), *((GetTileCenter() + GetTileExtent()).ToString()), *m_Url);

	// 	m_EstimatedGPUMemoryUsage = m_Actor->GetEstimatedGPUMemoryUsage();
	// 	m_HeapMemoryUsage = sizeof(UStreamingModelTile);
	// }

	// m_TileLoadState = ETileLoadState::Loaded;
}


GenericTwin::IStaticMeshComponent::EUploadMode UStreamingModelTile::AddToWorld(const FVector &Location)
{
	if(m_Actor)
	{
		m_Actor->SetActorLocation(Location);
	}
	else
	{
		m_Actor = GetWorld()->SpawnActor<AGenericTwinStaticMeshActor>(Location, FRotator::ZeroRotator, FActorSpawnParameters());
		OnTileVisibilityChanged();
		m_Actor->Initialize();
	}

	m_TileLoadState = ETileLoadState::Loaded;
	return m_Actor->GetUploadMode();
}


void UStreamingModelTile::SetScene(const GenericTwin::ScenePtr &ScenePtr)
{
	if(m_Actor)
	{
		m_Actor->SetFlattenedScene(ScenePtr);

		UE_LOG(LogStreamingModelTile, Log, TEXT("StreamingModelTile %s[%s] added for %s <-> %s Url: %s"), *(GetTileName()), *(m_TileId.ToString()), *((GetTileCenter() - GetTileExtent()).ToString()), *((GetTileCenter() + GetTileExtent()).ToString()), *m_Url);

		m_EstimatedGPUMemoryUsage = m_Actor->GetEstimatedGPUMemoryUsage();
		m_HeapMemoryUsage = sizeof(UStreamingModelTile);
	}

}
