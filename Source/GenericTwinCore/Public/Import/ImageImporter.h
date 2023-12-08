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

#include "Common/Image/Image.h"

#include <functional>

DECLARE_LOG_CATEGORY_EXTERN(LogImageImporter, Log, All);


namespace GenericTwin {

class IImageImporter;

struct SImageImportConfiguration;

class GENERICTWINCORE_API ImageImporter
{
public:

	typedef std::function< TSharedPtr<IImageImporter> () > CreateImporterFuncPtr;

	static void RegisterImporter(const FString &Extension, CreateImporterFuncPtr CreateImporter);
	static void UnregisterImporter(const FString &Extension);

	static SImagePtr ImportImageFromFile(const FString &Url, const SImageImportConfiguration &ImportConfiguration);

	static SImagePtr ImportImageFromMemory(const FString &UrlOrExtension, const TArray<uint8> Data, const SImageImportConfiguration &ImportConfiguration);

	static SImagePtr ImportImageFromMemory(const FString &UrlOrExtension, const void *Data, uint32 Size, const SImageImportConfiguration &ImportConfiguration);

private:

	static TMap<FString, CreateImporterFuncPtr>		s_RegisteredImporters;

};

}	//	namespace GenericTwin
