// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class GenericTwinCore : ModuleRules
{
	public GenericTwinCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",

				"ProceduralMeshComponent",
				"RealtimeMeshComponent",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	

				"GeometryCore",
				"GeometryFramework",

				"GeoReferencing",
				"Json",
				"ProceduralMeshComponent",
				"MeshDescription",
				"HTTP",
				"OpenSSL",


			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

		PublicIncludePaths.Add(Path.Combine("$(PluginDir)", "Source/ThirdParty/GSL/include"));
		PublicIncludePaths.Add(Path.Combine("$(PluginDir)", "Source/ThirdParty/RygsDXTc"));
		PublicIncludePaths.Add(Path.Combine("$(PluginDir)", "Source/ThirdParty/avir"));
		PublicIncludePaths.Add(Path.Combine("$(PluginDir)", "Source/ThirdParty/ConcaveHull"));
		PublicIncludePaths.Add(Path.Combine("$(PluginDir)", "Source/ThirdParty/earcut.hpp"));

	}
}
