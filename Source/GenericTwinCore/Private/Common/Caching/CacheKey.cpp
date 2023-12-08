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

#include "Common/Caching/CacheKey.h"

#include "Misc/SecureHash.h"

namespace GenericTwin
{

FCacheKey::FCacheKey()
	:	m_Key()
{
}

FCacheKey::FCacheKey(const FString& Url)
	:	m_Key(FMD5::HashAnsiString(*Url))
{
}

FCacheKey::FCacheKey(const TArray<uint8>& SourceData)
	:	m_Key(CalculateKey(SourceData.GetData(), SourceData.Num()))
{
}

FCacheKey::FCacheKey(const void* Data, int32 DataSize)
	:	m_Key(CalculateKey(Data, DataSize))
{
}

FCacheKey FCacheKey::FromKey(const FString &Key)
{
	FCacheKey CacheKey;
	CacheKey.m_Key = Key;
	return CacheKey;
}

FCacheKey FCacheKey::Calculate(const FString &Url)
{
	return FCacheKey(Url);
}

FCacheKey FCacheKey::Calculate(const TArray<uint8> &SourceData)
{
	return FCacheKey(SourceData);
}

FCacheKey FCacheKey::Calculate(const void *Data, int32 DataSize)
{
	return FCacheKey(Data, DataSize);
}

 
FString FCacheKey::CalculateKey(const void *Data, int32 DataSize)
{
	return FMD5::HashBytes( reinterpret_cast<const uint8*> (Data), DataSize);
}


}	//	namespace GenericTwin
