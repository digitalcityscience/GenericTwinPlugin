
#pragma once

#include "CoreMinimal.h"


namespace GenericTwin
{
	
class GENERICTWINCORE_API FPathUtils
{
public:

	static FString MakeAbsolutPath(const FString &InPath, const FString &BasePath);

};

} // namespace GenericTwin

