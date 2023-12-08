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

#include "Import/ImageImporter.h"
#include "Import/IImageImporter.h"

DEFINE_LOG_CATEGORY(LogImageImporter);

namespace GenericTwin {

TMap<FString, ImageImporter::CreateImporterFuncPtr> ImageImporter::s_RegisteredImporters;

void ImageImporter::RegisterImporter(const FString &Extension, CreateImporterFuncPtr CreateImporter)
{
	if(s_RegisteredImporters.Contains(Extension.ToLower()) == false)
	{
		s_RegisteredImporters.Add(Extension.ToLower(), CreateImporter);
	}
}

void ImageImporter::UnregisterImporter(const FString &Extension)
{
	if(s_RegisteredImporters.Contains(Extension.ToLower()))
	{
		s_RegisteredImporters.Remove(Extension.ToLower());
	}
}

SImagePtr ImageImporter::ImportImageFromFile(const FString &Url, const SImageImportConfiguration &ImportConfiguration)
{
	const FString extension = FPaths::GetExtension(Url, false).ToLower();

	SImagePtr image;

	if(s_RegisteredImporters.Contains(extension))
	{
		TSharedPtr<IImageImporter> importer = s_RegisteredImporters[extension]();

		if(importer)
		{
			UE_LOG(LogImageImporter, Verbose, TEXT("Using importer %s"), *(importer->GetImporterName()));

			image = importer->ImportImageFromFile(Url, ImportConfiguration);
		}
		else
		{
			UE_LOG(LogImageImporter, Warning, TEXT("Couldn't create importer for %s"), *extension);
		}
	}
	else
	{
		UE_LOG(LogImageImporter, Warning, TEXT("Didn't find importer for %s"), *extension);
	}

	return image;
}

SImagePtr ImageImporter::ImportImageFromMemory(const FString &UrlOrExtension, const TArray<uint8> Data, const SImageImportConfiguration &ImportConfiguration)
{
	return ImportImageFromMemory(UrlOrExtension, Data.GetData(), static_cast<uint32> (Data.Num()), ImportConfiguration);
}


SImagePtr ImageImporter::ImportImageFromMemory(const FString &UrlOrExtension, const void *Data, uint32 Size, const SImageImportConfiguration &ImportConfiguration)
{
	const FString extension = FPaths::GetExtension(UrlOrExtension, false).ToLower();

	SImagePtr image;

	if(s_RegisteredImporters.Contains(extension))
	{
		TSharedPtr<IImageImporter> importer = s_RegisteredImporters[extension]();

		if(importer)
		{
			UE_LOG(LogImageImporter, Log, TEXT("Using importer %s"), *(importer->GetImporterName()));

			image = importer->ImportImageFromMemory(UrlOrExtension, Data, Size, ImportConfiguration);
		}
		else
		{
			UE_LOG(LogImageImporter, Warning, TEXT("Couldn't create importer for %s"), *extension);
		}
	}
	else
	{
		UE_LOG(LogImageImporter, Warning, TEXT("Didn't find importer for %s"), *extension);
	}

	return image;
}

}	//	namespace GenericTwin
