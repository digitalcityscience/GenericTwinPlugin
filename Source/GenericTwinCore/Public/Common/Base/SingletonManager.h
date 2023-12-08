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

#include "Common/BaseTypes.h"

namespace GenericTwin
{

template<typename T>
class TSingletonManager
{

public:

	static bool Create(UObject *World, const FName &Name);
	static T& GetInstance();
	static void Destroy();

	virtual ~TSingletonManager() = 0
		{	}

protected:

	virtual void Shutdown() = 0
		{	}

	static T		*s_Instance;

};

template<typename T>
inline bool TSingletonManager<T>::Create(UObject *World, const FName &Name)
{
	bool created = false;
	if(s_Instance == 0)
	{
		s_Instance = NewObject<T>(World, Name);

		if(s_Instance)
		{
			s_Instance->AddToRoot();
			created = true;
		}
	}

	return created;
}

template<typename T>
inline T& TSingletonManager<T>::GetInstance()
{
	return *s_Instance;
}


template<typename T>
inline void TSingletonManager<T>::Destroy()
{
	if(s_Instance)
	{
		s_Instance->Shutdown();
		s_Instance->RemoveFromRoot();
		s_Instance->ConditionalBeginDestroy();
		s_Instance = 0;
	}
}

template< typename T >
T* TSingletonManager<T>::s_Instance = 0;

}	//	namespace GenericTwin
