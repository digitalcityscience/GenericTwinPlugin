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

#include "Common/Caching/CacheKey.h"
#include "Common/Caching/CachedFile.h"

namespace GenericTwin
{

class GENERICTWINCORE_API LocalFileCache
{
public:

	static LocalFileCache& GetInstance();
	static void Initialize(const FString &CachePath);
	static void Destroy();

	bool IsInitialized() const;

	void Add(const FCacheKey &Key, const TArray<uint8> &SourceData);
	void Add(const FCacheKey &Key, const void *Data, int32 DataSize);
	
	bool BeginFile(const FCacheKey &Key, const void *Data, int32 DataSize);
	void AddToFile(const FCacheKey &Key, const void *Data, int32 DataSize);


	SCachedFilePtr GetCachedFile(const FCacheKey &Key) const;

private:

	LocalFileCache();

	bool WriteToFile(const FCacheKey &Key, const void *Data, int32 DataSize, bool Append);

	void BuildFilePath(const FCacheKey &Key, FString &Path, FString &File);

	bool CreateDirectory(const FString &Path);

	bool								m_IsInitialized = false;

	FString								m_CachePath;

	TMap<FCacheKey, FString>			m_CachedFiles;

	static LocalFileCache				*s_Instance;

};

inline bool LocalFileCache::IsInitialized() const
{
	return m_IsInitialized;
}

}	//	namespace GenericTwin

