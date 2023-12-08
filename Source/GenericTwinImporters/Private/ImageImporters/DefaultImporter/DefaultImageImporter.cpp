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

#include "ImageImporters/DefaultImporter/DefaultImageImporter.h"
#include "Import/ImageImportConfiguration.h"

#include "Common/Image/Image.h"
#include "Common/Image/ImageResizer.h"

#include "IImageWrapperModule.h"

namespace GenericTwin {

TSharedPtr<IImageImporter> DefaultImageImporter::Create()
{
	return TSharedPtr<IImageImporter>(new DefaultImageImporter);
}

FString DefaultImageImporter::GetImporterName() const
{
	return TEXT("DefaultImageImporter");
}

SImagePtr DefaultImageImporter::ImportImageFromFile(const FString &Url, const SImageImportConfiguration &ImportConfiguration)
{
	TArray<uint8> rawData;
	if(FFileHelper::LoadFileToArray(rawData, *Url))
	{
		return ImportImageFromMemory(Url, rawData.GetData(), static_cast<uint32> (rawData.Num()), ImportConfiguration);
	}

	return SImagePtr();
}

SImagePtr DefaultImageImporter::ImportImageFromMemory(const FString &UrlOrExtension, const void *Data, uint32 Size, const SImageImportConfiguration &ImportConfiguration)
{
	SImagePtr image;

	IImageWrapperModule &imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> imageWrapper = imageWrapperModule.CreateImageWrapper(GetImageFormat(UrlOrExtension));

	if (imageWrapper.IsValid() && imageWrapper->SetCompressed(Data, Size))
	{
		image = SImagePtr(new SImage);
		uint8 *imgDataPtr = image ? image->Allocate(imageWrapper->GetWidth(), imageWrapper->GetHeight(), SImage::EFormat::RGBA) : 0;
		if(imgDataPtr)
		{
			imageWrapper->GetRaw(ERGBFormat::RGBA, 8, TArrayView<uint8, int32>(imgDataPtr, image->GetDataSize()));

			if	(	ImportConfiguration.MaxImageSize > 0
				&&	(image->GetWidth() > ImportConfiguration.MaxImageSize || image->GetHeight() > ImportConfiguration.MaxImageSize)
				)
			{
				image = ImageResizer::Resize(image, ImportConfiguration.MaxImageSize, true);
			}
		}
	}

	return image;
}

EImageFormat DefaultImageImporter::GetImageFormat(const FString &FileName) const
{
	EImageFormat format = EImageFormat::Invalid;

	FString extension = FPaths::GetExtension(FileName, false);
	extension = extension.ToLower();
	if (extension == "png")
		format = EImageFormat::PNG;
	else if (extension == "jpg")
		format = EImageFormat::JPEG;

	return format;
}


}	//	namespace GenericTwin

