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

#include "Streaming/TileStreamingLayer.h"
#include "Streaming/ITileIndex.h"

UTileStreamingLayer::UTileStreamingLayer()
{
}

void UTileStreamingLayer::OnEndPlay(FTwinWorldContext& TwinWorldCtx)
{
	Super::OnEndPlay(TwinWorldCtx);
}

void UTileStreamingLayer::ExtractStreamingParameters(const FJsonObject &Parameters, UTileManager::SConfiguration &Configuration)
{
	m_DrawTileBorders &= Parameters.TryGetBoolField(TEXT("draw_tile_borders"), m_DrawTileBorders);

	if(Parameters.HasTypedField<EJson::Number>(TEXT("view_distance")))
		Configuration.MaxViewDistance = Parameters.GetNumberField(TEXT("view_distance")) * 100.0;

	if(Parameters.HasTypedField<EJson::Number>(TEXT("view_rect_half_size")))
		Configuration.ViewRectHalfSize = Parameters.GetNumberField(TEXT("view_rect_half_size"));

	if (Parameters.HasTypedField<EJson::Number>(TEXT("max_heap_memory_mb")))
		Configuration.MaxHeapMemoroyUsage = Parameters.GetNumberField(TEXT("max_heap_memory_mb")) * 1024 * 1024;

	if(Parameters.HasTypedField<EJson::Number>(TEXT("max_gpu_memory_mb")))
		Configuration.MaxGPUMemoryUsage = Parameters.GetNumberField(TEXT("max_gpu_memory_mb")) * 1024 * 1024;

	if(Parameters.HasTypedField<EJson::Number>(TEXT("purge_interval_s")))
		Configuration.PurgeInterval = Parameters.GetNumberField(TEXT("purge_interval_s"));

}

ELayerCoverage UTileStreamingLayer::GetLayerCoverage(const FBox2d &Area) const
{
	return GetCoverage(Area);
}
