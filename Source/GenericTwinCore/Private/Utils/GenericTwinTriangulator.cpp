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

#include "Utils/GenericTwinTriangulator.h"

#include "earcut.hpp"

namespace GenericTwin {

TArray<int32> Triangulator::Triangulate(const TArray<FVector2D> &Polygon, bool ReverseIndices)
{
	mapbox::detail::Earcut<int32> tess;

	using Point = std::array<double, 2>;
	std::vector<Point> outline;
	for(const FVector2D &p : Polygon)
		outline.push_back({p.X, p.Y});

	std::vector<std::vector<Point>> polygon;
	polygon.push_back(outline);
	tess(polygon);

	TArray<int32> triangles;
	if(ReverseIndices)
	{
		for (std::vector<int32>::reverse_iterator rIter = tess.indices.rbegin(); rIter != tess.indices.rend(); ++rIter)
			triangles.Add(*rIter);
	}
	else
	{
		for (int32 idx : tess.indices)
			triangles.Add(idx);
	}

	return triangles;
}

}	//	namespace GenericTwin
