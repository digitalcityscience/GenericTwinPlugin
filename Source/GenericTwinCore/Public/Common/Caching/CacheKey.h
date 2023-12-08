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
#include "Containers/UnrealString.h"

// obviously, this forward declaration is required
uint32 GetTypeHash(const FString& S);

namespace GenericTwin
{

class GENERICTWINCORE_API FCacheKey
{
public:

	FCacheKey();

	explicit FCacheKey(const FString& Url);
	explicit FCacheKey(const TArray<uint8>& SourceData);
	explicit FCacheKey(const void* Data, int32 DataSize);

	static FCacheKey FromKey(const FString &Key);

	static FCacheKey Calculate(const FString &Url);
	static FCacheKey Calculate(const TArray<uint8> &SourceData);
	static FCacheKey Calculate(const void *Data, int32 DataSize);

	explicit operator bool() const;

	bool IsValid() const;
	const FString& AsString() const;

	void SetSlot(const FString& Slot);
	const FString& GetSlot() const;

	uint32 GetTypeHash() const;

	bool operator == (const FCacheKey &RHS) const;

private:

	static FString CalculateKey(const void *Data, int32 DataSize);

	FString				m_Key;
	FString				m_Slot = FString(TEXT("default"));

};

inline  FCacheKey::operator bool() const
{
	return IsValid();
}

inline bool FCacheKey::IsValid() const
{
	return m_Key.Len() == 32;
}

inline const FString& FCacheKey::AsString() const
{
	return m_Key;
}

inline void FCacheKey::SetSlot(const FString& Slot)
{
	m_Slot = Slot;
}

inline const FString& FCacheKey::GetSlot() const
{
	return m_Slot;
}

inline uint32 FCacheKey::GetTypeHash() const
{
	return ::GetTypeHash(m_Key);
}

inline bool FCacheKey::operator == (const FCacheKey &RHS) const
{
	return m_Key == RHS.m_Key;
}

}	//	namespace GenericTwin

inline uint32 GetTypeHash(const GenericTwin::FCacheKey& K)
{
	return K.GetTypeHash();
}
