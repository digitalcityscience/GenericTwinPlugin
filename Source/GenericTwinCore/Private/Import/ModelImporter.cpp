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

#include "Import/ModelImporter.h"
#include "Import/IModelImporter.h"

#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY(LogModelImporter);

namespace GenericTwin {

TMap<FString, ModelImporter::CreateImporterFuncPtr> ModelImporter::s_RegisteredImporters;

void ModelImporter::RegisterImporter(const FString &Extension, CreateImporterFuncPtr CreateImporter)
{
	if(s_RegisteredImporters.Contains(Extension.ToLower()) == false)
	{
		s_RegisteredImporters.Add(Extension.ToLower(), CreateImporter);
	}
}

void ModelImporter::UnregisterImporter(const FString &Extension)
{
	if(s_RegisteredImporters.Contains(Extension.ToLower()))
	{
		s_RegisteredImporters.Remove(Extension.ToLower());
	}
}

ScenePtr ModelImporter::ImportModel(const FString &PathToModel, const SModelImportConfiguration &ImportConfiguration)
{
	const FString extension = FPaths::GetExtension(PathToModel, false).ToLower();

	ScenePtr scenePtr;

	if(s_RegisteredImporters.Contains(extension))
	{
		IModelImporter *importer = s_RegisteredImporters[extension]();

		if(importer)
		{
			UE_LOG(LogModelImporter, Verbose, TEXT("Using importer %s"), *(importer->GetImporterName()));

			scenePtr = importer->ImportModel(PathToModel, ImportConfiguration);
			delete importer;
		}
		else
		{
			UE_LOG(LogModelImporter, Warning, TEXT("Couldn't create importer for %s"), *extension);
		}
	}
	else
	{
		UE_LOG(LogModelImporter, Warning, TEXT("Didn't find importer for %s"), *extension);
	}

	return scenePtr;
}

}	//	namespace GenericTwin
