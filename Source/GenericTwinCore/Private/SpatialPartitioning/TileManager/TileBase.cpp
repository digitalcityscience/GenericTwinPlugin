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


#include "SpatialPartitioning/TileManager/TileBase.h"

UTileBase::~UTileBase()
{
}

void UTileBase::Initialize(const FVector &Location, const FVector2D &TileSize)
{
	m_WorldLocation = Location;
	m_TileSize = TileSize;

	UpdateBoundings(0.0, 20000.0);
}

void UTileBase::Shutdown()
{
}

bool UTileBase::Load()
{
	return false;
}

void UTileBase::Unload()
{
}

void UTileBase::UpdateLoading(float DeltaSeconds)
{
}

void UTileBase::GetMemoryUsage(uint32 &HeapUsage, uint32 &EstimatedGPUUsage)
{
	HeapUsage = m_HeapMemoryUsage;
	EstimatedGPUUsage = m_EstimatedGPUMemoryUsage;
}

void UTileBase::SetIsHidden(bool IsHidden)
{
}

void UTileBase::UpdateTileVisibility(float NewVisibility)
{
	if(NewVisibility != m_TileVisibility)
	{
		m_TileVisibility = NewVisibility;
		OnTileVisibilityChanged();
	}
}

void UTileBase::OnTileVisibilityChanged()
{
}

FString UTileBase::GetTileName() const
{
	return FString(TEXT("TileBase"));
}

void UTileBase::UpdateBoundings(double MinZ, double MaxZ)
{
	m_TileCenter.X = m_WorldLocation.X + 0.5 * m_TileSize.X;
	m_TileCenter.Y = m_WorldLocation.Y - 0.5 * m_TileSize.Y;
	m_TileCenter.Z = 0.5 * (MinZ + MaxZ);

	m_TileExtent.X = 0.5 * m_TileSize.X;
	m_TileExtent.Y = 0.5 * m_TileSize.Y;
	m_TileExtent.Z = 0.5 * (MaxZ - MinZ);
}
