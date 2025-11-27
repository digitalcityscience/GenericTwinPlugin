
#include "Utils/GenericTwinCoreUtils.h"

namespace GenericTwin {

bool CoreUtils::IsRunningFromEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

FString CoreUtils::GetProjectDir()
{
	return IsRunningFromEditor() ? FPaths::ProjectDir() : FPaths::LaunchDir();
}

}	//	namespace GenericTwin
