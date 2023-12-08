
#include "Utils/PathUtils.h"

namespace GenericTwin
{

FString FPathUtils::MakeAbsolutPath(const FString &InPath, const FString &BasePath)
{
	if(FPaths::IsRelative(InPath))
	{
		FString path;
		FString fileName;
		FString extension;
		FPaths::Split(BasePath, path, fileName, extension);
		return FPaths::Combine(path, InPath);
	}

	return InPath;
}

} // namespace GenericTwin
