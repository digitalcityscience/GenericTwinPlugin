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

#include "Configuration/Configs/LocalFileCacheConfig.h"

#include "Dom/JsonObject.h"

FString SLocalFileCacheConfig::ConfigurationName = FString(TEXT("local_file_cache"));
bool SLocalFileCacheConfig::IsMandatory = false;

TSharedPtr<SGenericConfigurationBase> SLocalFileCacheConfig::parse(TSharedPtr<FJsonObject> &jsonObj)
{
	const TSharedPtr<FJsonObject> *cfgObj = 0;
	if	(	jsonObj->TryGetObjectField(ConfigurationName, cfgObj)
		&&	cfgObj
		&&	(*cfgObj)->HasTypedField<EJson::String>("cache_path")
		)
	{
		TSharedPtr<SLocalFileCacheConfig> cfg(new SLocalFileCacheConfig);

		if (cfg)
		{
			cfg->CachePath = ((*cfgObj)->GetStringField("cache_path"));
		}

		return cfg;
	}

	return TSharedPtr<SGenericConfigurationBase>();
}
