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

#include "Common/Scene/Visitors/CalcSceneBoundingsVisitor.h"

#include "Math/NumericLimits.h"


namespace GenericTwin
{

CalcSceneBoundingsVisitor::CalcSceneBoundingsVisitor()
	:	m_minExtent(TNumericLimits<double>::Max(), TNumericLimits<double>::Max(), TNumericLimits<double>::Max())
	,	m_maxExtent(TNumericLimits<double>::Min(), TNumericLimits<double>::Min(), TNumericLimits<double>::Min())
{
}

bool CalcSceneBoundingsVisitor::VisitNode(const SScene &Scene, SSceneNode &Node)
{
	for(const SSceneMesh *mesh : Node.Meshes)
	{
		for(const FVector &pos : mesh->Vertices)
		{
			if(pos.X < m_minExtent.X) m_minExtent.X = pos.X;
			if(pos.Y < m_minExtent.Y) m_minExtent.Y = pos.Y;
			if(pos.Z < m_minExtent.Z) m_minExtent.Z = pos.Z;

			if(pos.X > m_maxExtent.X) m_maxExtent.X = pos.X;
			if(pos.Y > m_maxExtent.Y) m_maxExtent.Y = pos.Y;
			if(pos.Z > m_maxExtent.Z) m_maxExtent.Z = pos.Z;
		}
	}
	return true;
}


}	//	namespace GenericTwin

