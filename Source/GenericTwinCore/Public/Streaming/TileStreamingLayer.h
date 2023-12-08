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

#include "Layers/TileLayerBase.h"
#include "TileStreamingLayer.generated.h" 

namespace GenericTwin {

class ITileIndex;

}	//	namespace GenericTwin

class UTileManager;

UCLASS()
class GENERICTWINCORE_API UTileStreamingLayer	:	public UTileLayerBase
{
	GENERATED_BODY()

	typedef UTileLayerBase Super;

public:

	UTileStreamingLayer();

	virtual void OnEndPlay(FTwinWorldContext& TwinWorldCtx) override;

	/*
	*	ULayerBase
	*/
	virtual ELayerCoverage GetLayerCoverage(const FBox2d &Area) const override;

	void SetTileIndex(GenericTwin::ITileIndex &TileIndex);

protected:

	void ExtractStreamingParameters(const FJsonObject &Parameters, UTileManager::SConfiguration &Configuration);

	GenericTwin::ITileIndex			*m_TileIndex = 0;

};


inline void UTileStreamingLayer::SetTileIndex(GenericTwin::ITileIndex &TileIndex)
{
	m_TileIndex = &TileIndex;
}
