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

#include "Configuration/Configs/ControllersConfig.h"

#include "Dom/JsonObject.h"

FString SControllersConfig::ConfigurationName = FString(TEXT("controllers"));
bool SControllersConfig::IsMandatory = false;

TSharedPtr<SGenericConfigurationBase> SControllersConfig::parse(TSharedPtr<FJsonObject> &jsonObj)
{
	const TSharedPtr<FJsonObject> *cfgObj = 0;
	if	(	jsonObj->TryGetObjectField(ConfigurationName, cfgObj)
		&&	cfgObj		
		)
	{
		TSharedPtr<SControllersConfig> cfg(new SControllersConfig);

		if (cfg)
		{
			if((*cfgObj)->HasTypedField<EJson::String>("base_controller"))
				cfg->BaseController = ((*cfgObj)->GetStringField("base_controller"));

			if ((*cfgObj)->HasTypedField<EJson::Array>("controllers"))
			{
				const TArray < TSharedPtr < FJsonValue > >* values = 0;
				(*cfgObj)->TryGetArrayField(TEXT("controllers"), values);

				for (const auto& value : (*values))
				{
					const TSharedPtr < FJsonObject >* valueObject = 0;
					if (value->TryGetObject(valueObject)
						&& valueObject
						&& (*valueObject)->HasTypedField<EJson::String>("controller_id")
						&& (*valueObject)->HasTypedField<EJson::String>("controller_reference")
						)
					{
						SControllerConfig ctrlCfg;

						ctrlCfg.ControllerId = ((*valueObject)->GetStringField("controller_id"));
						ctrlCfg.ControllerReference = ((*valueObject)->GetStringField("controller_reference"));

						cfg->Controllers.Add(ctrlCfg);

					}
				}
			}

		}

		return cfg;
	}

	return TSharedPtr<SGenericConfigurationBase>();
}
