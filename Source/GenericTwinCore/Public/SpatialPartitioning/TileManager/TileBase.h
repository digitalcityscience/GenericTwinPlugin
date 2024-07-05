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

#include "UObject/NoExportTypes.h"

#include "TileBase.generated.h"

enum class ETileLoadState : uint8
{
	Unknown,
	Loading,
	Loaded,
	Unloading,
	Empty,

};

UCLASS()
class GENERICTWINCORE_API UTileBase	:	public UObject
{
	GENERATED_BODY()

public:

	UTileBase();
	UTileBase(const GenericTwin::TTileId &TileId);
	
	virtual ~UTileBase();

	void Initialize(const FVector &Location, const FVector2D &TileSize);

	virtual void Shutdown();
	
	virtual bool Load();
	virtual void Unload();

	virtual void GetMemoryUsage(uint32 &HeapUsage, uint32 &EstimatedGPUUsage);
	
	virtual void SetIsHidden(bool IsHidden);

	void UpdateTileVisibility(float NewVisibility);
	virtual void OnTileVisibilityChanged();

	virtual FString GetTileName() const;

	float GetTileVisibility() const;

	ETileLoadState GetTileLoadState() const;

	const FVector& GetTileCenter() const;
	const FVector& GetTileExtent() const;
	const FVector& GetWorldLocation() const;

	void Touch(const FDateTime &CurrentTime);
	void MarkAsEmpty();
	const FDateTime& GetLastTouched() const;

	void SetTileId(const GenericTwin::TTileId &TileId);
	const GenericTwin::TTileId& GetTileId() const;


protected:

	void UpdateBoundings(double MinZ, double MaxZ);

	GenericTwin::TTileId			m_TileId;
	ETileLoadState					m_TileLoadState = ETileLoadState::Unknown;
	float							m_TileVisibility = 0.0f;

	FDateTime						m_LastTouched;

	FVector							m_WorldLocation;
	FVector2D						m_TileSize;

	FVector							m_TileCenter;
	FVector							m_TileExtent;

	uint32							m_HeapMemoryUsage = 0;
	uint32							m_EstimatedGPUMemoryUsage = 0;

};

inline UTileBase::UTileBase()
	:	m_TileId()
	,	m_LastTouched()
{
}

inline UTileBase::UTileBase(const GenericTwin::TTileId &TileId)
	:	m_TileId(TileId)
	,	m_LastTouched()
{	
}

inline void UTileBase::SetTileId(const GenericTwin::TTileId &TileId)
{
	m_TileId = TileId;
}

inline const GenericTwin::TTileId& UTileBase::GetTileId() const
{
	return m_TileId;
}

inline ETileLoadState UTileBase::GetTileLoadState() const
{
	return m_TileLoadState;
}

inline const FVector& UTileBase::GetTileCenter() const
{
	return m_TileCenter;
}

inline const FVector& UTileBase::GetTileExtent() const
{
	return m_TileExtent;
}

inline const FVector& UTileBase::GetWorldLocation() const
{
	return m_WorldLocation;
}

inline float UTileBase::GetTileVisibility() const
{
	return m_TileVisibility;
}

inline void UTileBase::Touch(const FDateTime &CurrentTime)
{
	m_LastTouched = CurrentTime;
}

inline void UTileBase::MarkAsEmpty()
{
	m_TileLoadState = ETileLoadState::Empty;
}

inline const FDateTime& UTileBase::GetLastTouched() const
{
	return m_LastTouched;
}
