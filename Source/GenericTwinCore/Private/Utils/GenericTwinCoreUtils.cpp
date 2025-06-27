
#include "Utils/GenericTwinCoreUtils.h"


bool GenericTwinCoreUtils::IsRunningFromEditor()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}

FString GenericTwinCoreUtils::GetProjectDir()
{
	return GenericTwinCoreUtils::IsRunningFromEditor() ? FPaths::ProjectDir() : FPaths::LaunchDir();
}
