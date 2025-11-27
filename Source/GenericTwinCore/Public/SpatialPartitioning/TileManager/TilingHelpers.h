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

#include "SpatialPartitioning/TileManager/TileManagerTypes.h"

#include "SpatialPartitioning/TileManager/ILocationMapper.h"

namespace GenericTwin {

class GENERICTWINCORE_API TilingHelpers
{

public:

	static FIntVector2 MapLocationToTileXY(const FVector &Location, const FVector &RefPos, const FVector2D &TileSize);

	static FVector MapTileXYToLocation(const FIntVector2 &TileXY, const FVector &RefPos, const FVector2D &TileSize);

};

class GENERICTWINCORE_API BasicLocationMapper	:	public	ILocationMapper
{
public:

	BasicLocationMapper(const FVector &ReferencePosition, const FVector2D &TileSize);

	virtual TTileId MapLocationToTileId(const FVector &Location) override;

	virtual FVector MapTileIdToLocation(const TTileId &TileId) override;

	virtual FVector GetTileCenter(const TTileId &TileId) const override;

	virtual FBox2d GetTileArea(const TTileId &TileId) const override;

	virtual FVector2D GetTileSize() const override;

	virtual int32 GetTotalTileCount() const override;

private:

	FVector				m_ReferencePosition;
	FVector2D			m_TileSize;

};

class GENERICTWINCORE_API AreaLocationMapper	:	public	ILocationMapper
{
public:

	AreaLocationMapper(const FVector &LowerLeft, const FVector &UpperRight, const FVector2D &TileSize);

	void SetArea(const FVector &LowerLeft, const FVector &UpperRight);

	virtual TTileId MapLocationToTileId(const FVector &Location) override;

	virtual FVector MapTileIdToLocation(const TTileId &TileId) override;

	virtual FVector GetTileCenter(const TTileId &TileId) const override;

	virtual FBox2d GetTileArea(const TTileId &TileId) const override;

	virtual FVector2D GetTileSize() const override;

	virtual int32 GetTotalTileCount() const override;

	bool IsTileContained(const TTileId &TileId) const;

	void Reset();

private:

	FVector				m_LowerLeft;
	FVector				m_AreaSize;

	FVector2D			m_TileSize;

};

}	//	namespace GenericTwin 
