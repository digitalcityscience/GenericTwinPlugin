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


#include "Common/Scene/ISceneNodeVisitor.h"

namespace GenericTwin
{

struct SSceneNode;

class GENERICTWINCORE_API FlattenSceneVisitor	:	public ISceneNodeVisitor
{
public:

	FlattenSceneVisitor(const FTransform &Transform);
	virtual ~FlattenSceneVisitor();

	virtual void BeginVisitScene(const SScene &Scene) override;

	virtual bool VisitNode(const SScene &Scene, SSceneNode &Node) override;

	GenericTwin::ScenePtr GetFlattenedScene() const;

private:

	FTransform					m_Transform;

	GenericTwin::ScenePtr		m_FlattenedScene;

	TMap<FString, SSceneMesh*>	m_Meshes;

};

inline GenericTwin::ScenePtr FlattenSceneVisitor::GetFlattenedScene() const
{
	return m_FlattenedScene;
}

}	//	namespace GenericTwin

