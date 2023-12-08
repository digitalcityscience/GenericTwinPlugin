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

#include "Configuration/LayersConfiguration.h"

#include "Dom/JsonObject.h"

FString SLayersConfiguration::ConfigurationName = FString(TEXT("layers"));
bool SLayersConfiguration::IsMandatory = true;

TSharedPtr<SGenericConfigurationBase> SLayersConfiguration::parse(TSharedPtr<FJsonObject> &jsonObj)
{
	const TArray < TSharedPtr < FJsonValue > > *values = 0;
	if	(	jsonObj->TryGetArrayField(ConfigurationName, values)
		&&	values
		)
	{
		TSharedPtr<SLayersConfiguration> cfg(new SLayersConfiguration);

		if (cfg)
		{
			for(const auto &value : (*values))
			{
				const TSharedPtr < FJsonObject > *valueObject = 0;
				if	(	value->TryGetObject(valueObject)
					&&	valueObject
					&&	(*valueObject)->HasTypedField<EJson::String>("layer_type")
					)
				{
					SLayerConfiguration layerCfg;

					layerCfg.Type = ((*valueObject)->GetStringField("layer_type"));

					if ((*valueObject)->HasTypedField<EJson::String>("custom_name"))
						layerCfg.CustomName = ((*valueObject)->GetStringField("custom_name"));

					if ((*valueObject)->HasTypedField<EJson::Object>("dependency"))
						layerCfg.Dependency = (*valueObject)->GetObjectField("dependency");

					if ((*valueObject)->HasTypedField<EJson::Object>("parameters"))
						layerCfg.Parameters = (*valueObject)->GetObjectField("parameters");



					cfg->Layers.Add(layerCfg);

				}
			}

			return cfg;
		}
	}

	return TSharedPtr<SGenericConfigurationBase>();
}
