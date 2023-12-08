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

namespace GenericTwin {

class TTileId
{
public:

	TTileId()
	{	}

	TTileId(uint64 TileId)
		:	m_TileId(TileId)
	{	}

	TTileId(int32 TileX, int32 TileY, uint8 Level = 0)
		:	m_TileId(0)
	{
		Set(TileX, TileY);
	}

	static TTileId FromLocation(const FVector &Location, const FVector &RefPos, const FVector2D &TileSize)
	{
		FVector Delta(Location - RefPos);
		return TTileId(static_cast<int32> (Delta.X / TileSize.X), static_cast<int32> (-Delta.Y / TileSize.Y));
	}

	void Set(int32 TileX, int32 TileY, uint8 Level = 0)
	{
		m_TileId = (static_cast<uint64> (TileY) << 32) | (static_cast<uint64> (TileX) & 0xFFFFFFFF);
	}

	int32 GetTileX() const
	{
		return static_cast<int32> (m_TileId & 0xFFFFFFFF);
	}

	int32 GetTileY() const
	{
		return static_cast<int32> ((m_TileId >> 32) & 0xFFFFFFFF);
	}

	uint32 GetLevel() const
	{
		return 0;
	}

	void Get(int32 &TileX, int32 &TileY) const
	{
		TileX = static_cast<int32> (m_TileId & 0xFFFFFFFF);
		TileY = static_cast<int32> ((m_TileId >> 32) & 0xFFFFFFFF);
	}

	FIntVector2 Get() const
	{
		return FIntVector2(static_cast<int32> (m_TileId & 0xFFFFFFFF), static_cast<int32> ((m_TileId >> 32) & 0xFFFFFFFF));

	}

	FVector ToLocation(const FVector& RefPos, const FVector2D& TileSize) const
	{
		FVector Delta(TileSize.X * static_cast<double> (GetTileX()), -TileSize.Y * static_cast<double> (GetTileY()), 0.0);
		return RefPos + Delta;
	}

	bool operator == (const TTileId &Other) const
	{
		return m_TileId == Other.m_TileId;
	}

	bool operator != (const TTileId& Other) const
	{
		return m_TileId != Other.m_TileId;
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%llu"), m_TileId);
	}

private:

	uint64		m_TileId = 0;
};

}	//	namespace GenericTwin 


FORCEINLINE uint32 GetTypeHash(const GenericTwin::TTileId &TileId)
{
	uint32 Hash = FCrc::MemCrc32(&TileId, sizeof(GenericTwin::TTileId));
	return Hash;
}


/*
	Some forward decalarations
*/
class UTileBase;
class TileManager;


typedef TObjectPtr<UTileBase>	TileBasePtr;

DECLARE_DELEGATE_RetVal_OneParam(GenericTwin::TTileId, FMapLocationToTileId, const FVector&);
