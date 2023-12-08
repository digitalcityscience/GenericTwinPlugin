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

#include "Common/Caching/LocalFileCache.h"

#include "Misc/SecureHash.h"

#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"

#include <fstream>

namespace GenericTwin
{

LocalFileCache *LocalFileCache::s_Instance = 0;

LocalFileCache& LocalFileCache::GetInstance()
{
	if(s_Instance == 0)
	{
		s_Instance = new LocalFileCache;
	}

	return *s_Instance;
}


void LocalFileCache::Destroy()
{
	if(s_Instance)
	{
		delete s_Instance;
		s_Instance = 0;
	}
}

LocalFileCache::LocalFileCache()
{
}

void LocalFileCache::Initialize(const FString &CachePath)
{
	IFileManager &fileMgr = IFileManager::Get();

	class Visitor : public IPlatformFile::FDirectoryVisitor
	{
	public:

		Visitor(TMap<FCacheKey, FString> &CachedFiles)
			:	m_CachedFiles(CachedFiles)
		{
		}

		bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			FString path;
			FString file;
			FString ext;
			FPaths::Split(FilenameOrDirectory, path, file, ext);

			if	(	bIsDirectory == false
				&&	file.Len() == 24
				)
			{
				FString cacheKey = path.Right(11).Replace(TEXT("/"), TEXT("")) + file;
				m_CachedFiles.Add(FCacheKey::FromKey(cacheKey), FilenameOrDirectory);
			}

			return true;
		}

	private:

		TMap<FCacheKey, FString>		&m_CachedFiles;
	};

	LocalFileCache &lfc = GetInstance();

	lfc.m_CachePath = CachePath;

	Visitor visitor(lfc.m_CachedFiles);

	fileMgr.IterateDirectoryRecursively(*lfc.m_CachePath, visitor);
	lfc.m_IsInitialized = true;
}

void LocalFileCache::Add(const FCacheKey &Key, const TArray<uint8> &SourceData)
{
}

void LocalFileCache::Add(const FCacheKey &Key, const void *Data, int32 DataSize)
{
}

bool LocalFileCache::BeginFile(const FCacheKey &Key, const void *Data, int32 DataSize)
{
	return m_IsInitialized && Key.IsValid() ? WriteToFile(Key, Data, DataSize, false) : false;
}

void LocalFileCache::AddToFile(const FCacheKey &Key, const void *Data, int32 DataSize)
{
	if(m_IsInitialized && DataSize && Key.IsValid())
		WriteToFile(Key, Data, DataSize, true);
}

bool LocalFileCache::WriteToFile(const FCacheKey &Key, const void *Data, int32 DataSize, bool Append)
{
	bool res = false;
	FString path;
	FString file;
	BuildFilePath(Key, path, file);

	if (CreateDirectory(path))
	{
		FString fullPath = FPaths::Combine(path, file);
		std::ofstream out;
		out.open(TCHAR_TO_ANSI(*fullPath), std::ofstream::out | std::ofstream::binary | (Append ? std::ofstream::app : std::ofstream::trunc));
		if (out.is_open())
		{
			out.write(reinterpret_cast<const char*> (Data), DataSize);

			out.close();
			res = true;
		}
	}
	return res;
}

bool LocalFileCache::CreateDirectory(const FString &Path)
{
	IPlatformFile  &PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	return PlatformFile.CreateDirectoryTree(*Path);
}

void LocalFileCache::BuildFilePath(const FCacheKey &Key, FString &Path, FString &File)
{
	const FString &keyStr = Key.AsString();

	FString levels[4];
	for(signed i = 0; i < 4; ++i)
	{
		levels[i] += keyStr[2 * i];
		levels[i] += keyStr[2 * i + 1];
	}
	
	Path = FPaths::Combine(m_CachePath, Key.GetSlot(), levels[0], levels[1], levels[2], levels[3]);
	File = keyStr.Right(24);
}

SCachedFilePtr LocalFileCache::GetCachedFile(const FCacheKey &Key) const
{
	if(m_CachedFiles.Contains(Key))
	{
		const FString& cachedFileName = m_CachedFiles[Key];
		IFileManager &fileMgr = IFileManager::Get();
		const int64 fileSize = fileMgr.FileSize(*cachedFileName);

		if(fileSize != INDEX_NONE )
		{
			std::ifstream in;
			in.open(TCHAR_TO_ANSI(*cachedFileName), std::ios::binary);
			if(in.is_open())
			{
				SCachedFilePtr cachedFile(new SCachedFile());

				if(cachedFile)
				{
					void *dataPtr = new uint8[fileSize];
					if(dataPtr)
					{
						cachedFile->Data = dataPtr;
						cachedFile->DataSize = fileSize;
						in.seekg(0, std::ios::beg);
						in.read(reinterpret_cast<char*> (dataPtr), fileSize);
					}
				}
				in.close();

				return cachedFile;
			}
		}


	}
	return SCachedFilePtr();
}


}	//	namespace GenericTwin

