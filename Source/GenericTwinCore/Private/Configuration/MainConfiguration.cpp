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

#include "Configuration/MainConfiguration.h"
#include "Common/Configuration/GenericConfiguration.h"

#include "Configuration/StartLocationConfiguration.h"
#include "Configuration/LayersConfiguration.h"
#include "Configuration/GeoReferenceConfiguration.h"
#include "Configuration/Configs/PawnConfig.h"
#include "Configuration/Configs/ControllersConfig.h"

#include "Common/Caching/LocalFileCache.h"
#include "Common/Material/GenericTwinMaterialManager.h"

#include "Utils/FilePathUtils.h"

#include "GeoReferencingSystem.h"


bool MainConfiguration::loadConfiguration(GenericConfiguration &Configuration, const FString &ConfigFileName)
{
	bool res = false;

	if(Configuration.loadConfiguration(ConfigFileName))
	{
		res = Configuration.parse<SGeoReferenceConfiguration>();
		res &= Configuration.parse<SStartLocationConfiguration>();
		res &= Configuration.parse<SLayersConfiguration>();
		res &= Configuration.parse<SLocalFileCacheConfig>();
		res &= Configuration.parse<SPawnConfig>();
		res &= Configuration.parse<SControllersConfig>();
	}

	return res;
}


void MainConfiguration::configure(AGeoReferencingSystem &GeoRefSystem, GenericConfiguration &Configuration)
{
	const SGeoReferenceConfiguration &geoRefCfg = Configuration.getConfiguration<SGeoReferenceConfiguration>();

	GeoRefSystem.bOriginLocationInProjectedCRS = false;
	GeoRefSystem.OriginLongitude = geoRefCfg.Origin.X;
	GeoRefSystem.OriginLatitude = geoRefCfg.Origin.Y;
	GeoRefSystem.OriginAltitude = geoRefCfg.Origin.Z;

	GeoRefSystem.ProjectedCRS = geoRefCfg.ProjectedCRS;
	GeoRefSystem.PlanetShape = geoRefCfg.IsPlanetShape ? EPlanetShape::RoundPlanet : EPlanetShape::FlatPlanet;

	GeoRefSystem.ApplySettings();

}

void MainConfiguration::configureLocalFileCache(GenericConfiguration& Configuration)
{
	if(Configuration.hasConfiguration<SLocalFileCacheConfig>())
	{
		const SLocalFileCacheConfig &cfg = Configuration.getConfiguration<SLocalFileCacheConfig>();
		GenericTwin::LocalFileCache::Initialize(GenericTwin::FPathUtils::MakeAbsolutPath(cfg.CachePath, FPaths::LaunchDir()));
	}
}

void MainConfiguration::configureMaterialManager(GenericConfiguration& Configuration)
{
	FString mapFile = Configuration.getString(TEXT("material_map"));
	if(mapFile.IsEmpty() == false)
	{
		UGenericTwinMaterialManager::GetInstance().LoadGenericMapping( GenericTwin::FPathUtils::MakeAbsolutPath(mapFile, Configuration.GetConfigurationFileName()) );
	}
}
