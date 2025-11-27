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

#include "SpatialPartitioning/TileManager/TilingHelpers.h"

namespace GenericTwin {

FIntVector2 TilingHelpers::MapLocationToTileXY(const FVector &Location, const FVector &RefPos, const FVector2D &TileSize)
{
	FVector Delta(Location - RefPos);
	return FIntVector2(static_cast<int32> (Delta.X / TileSize.X), static_cast<int32> (-Delta.Y / TileSize.Y));
}

FVector TilingHelpers::MapTileXYToLocation(const FIntVector2 &TileXY, const FVector &RefPos, const FVector2D &TileSize)
{
	FVector Delta(TileSize.X * static_cast<double> (TileXY.X), -TileSize.Y * static_cast<double> (TileXY.Y), 0.0);
	return RefPos + Delta;
}


BasicLocationMapper::BasicLocationMapper(const FVector &ReferencePosition, const FVector2D &TileSize)
	:	m_ReferencePosition(ReferencePosition)
	,	m_TileSize(TileSize)
{
}

TTileId BasicLocationMapper::MapLocationToTileId(const FVector &Location)
{
	const FIntVector2  tileXY = TilingHelpers::MapLocationToTileXY(Location, m_ReferencePosition, m_TileSize);
	return TTileId(tileXY.X, tileXY.Y);
}

FVector BasicLocationMapper::MapTileIdToLocation(const TTileId &TileId)
{
	return TilingHelpers::MapTileXYToLocation(TileId.Get(), m_ReferencePosition, m_TileSize);
}

FVector BasicLocationMapper::GetTileCenter(const TTileId &TileId) const
{
	FVector tileCtr = TilingHelpers::MapTileXYToLocation(TileId.Get(), m_ReferencePosition, m_TileSize);
	tileCtr + FVector(0.5 * m_TileSize.X, -0.5 * m_TileSize.Y, 0.0);

	return tileCtr;
}

FBox2d BasicLocationMapper::GetTileArea(const TTileId &TileId) const
{
	const FVector tilePos = TilingHelpers::MapTileXYToLocation(TileId.Get(), m_ReferencePosition, m_TileSize);
	FBox2d area(FVector2D(tilePos.X, tilePos.Y), FVector2D(tilePos.X, tilePos.Y) + m_TileSize);
	return area;
}

FVector2D BasicLocationMapper::GetTileSize() const
{
	return m_TileSize;
}

int32 BasicLocationMapper::GetTotalTileCount() const
{
	return -1;
}

AreaLocationMapper::AreaLocationMapper(const FVector &LowerLeft, const FVector &UpperRight, const FVector2D &TileSize)
	:	m_LowerLeft(LowerLeft)	//	m_LowerLeft(0.0, 0.0, LowerLeft.Z)
	,	m_AreaSize(FMath::Abs(UpperRight.X - LowerLeft.X), FMath::Abs(UpperRight.Y - LowerLeft.Y), UpperRight.Z - LowerLeft.Z)
	,	m_TileSize(TileSize)
{
}

void AreaLocationMapper::SetArea(const FVector &LowerLeft, const FVector &UpperRight)
{
	m_LowerLeft = LowerLeft;
	m_AreaSize = FVector(FMath::Abs(UpperRight.X - LowerLeft.X), FMath::Abs(UpperRight.Y - LowerLeft.Y), UpperRight.Z - LowerLeft.Z);
}


TTileId AreaLocationMapper::MapLocationToTileId(const FVector &Location)
{
	const FIntVector2  tileXY = TilingHelpers::MapLocationToTileXY(Location, m_LowerLeft, m_TileSize);
	return TTileId(tileXY.X, tileXY.Y);
}

FVector AreaLocationMapper::MapTileIdToLocation(const TTileId &TileId)
{
	return TilingHelpers::MapTileXYToLocation(TileId.Get(), m_LowerLeft, m_TileSize);
}

FVector AreaLocationMapper::GetTileCenter(const TTileId &TileId) const
{
	FVector tileCtr = TilingHelpers::MapTileXYToLocation(TileId.Get(), m_LowerLeft, m_TileSize);
	tileCtr + FVector(0.5 * m_TileSize.X, -0.5 * m_TileSize.Y, 0.0);

	return tileCtr;
}

FBox2d AreaLocationMapper::GetTileArea(const TTileId &TileId) const
{
	// const FVector tilePos = TilingHelpers::MapTileXYToLocation(TileId.Get(), FVector(0.0, 0.0, m_LowerLeft.Z), m_TileSize);
	const FVector tilePos = TilingHelpers::MapTileXYToLocation(TileId.Get(), m_LowerLeft, m_TileSize);
	// FBox2d area(FVector2D(tilePos.X, tilePos.Y), FVector2D(tilePos.X, tilePos.Y) + m_TileSize);
	FBox2d area(FVector2D(tilePos.X, tilePos.Y - m_TileSize.Y), FVector2D(tilePos.X + m_TileSize.X, tilePos.Y));
	return area;
}

FVector2D AreaLocationMapper::GetTileSize() const
{
	return m_TileSize;
}

int32 AreaLocationMapper::GetTotalTileCount() const
{
	return FMath::Floor(m_AreaSize.X / m_TileSize.X) * FMath::Floor(m_AreaSize.Y / m_TileSize.Y);
}

void AreaLocationMapper::Reset()
{
	m_LowerLeft = FVector::ZeroVector;
	m_AreaSize = FVector::ZeroVector;
}

bool AreaLocationMapper::IsTileContained(const TTileId &TileId) const
{
	FBox2d tileArea = GetTileArea(TileId);

	// const bool outside =	tileArea.Max.X < 0.0
	// 					||	tileArea.Min.X > m_AreaSize.X
	// 					||	tileArea.Max.Y < 0.0
	// 					||	tileArea.Min.Y > m_AreaSize.Y;

	// const bool outside =	tileArea.Max.X <= m_LowerLeft.X
	// 					||	tileArea.Min.X >= (m_LowerLeft.X + m_AreaSize.X)
	// 					||	tileArea.Max.Y <= m_LowerLeft.Y
	// 					||	tileArea.Min.Y >= (m_LowerLeft.Y + m_AreaSize.Y);

	const bool outside =	tileArea.Max.X <= m_LowerLeft.X
						||	tileArea.Min.X >= (m_LowerLeft.X + m_AreaSize.X)
						||	tileArea.Max.Y <= (m_LowerLeft.Y - m_AreaSize.Y)
						||	tileArea.Min.Y >= m_LowerLeft.Y;

	return !outside;
}

}	//	namespace GenericTwin 
