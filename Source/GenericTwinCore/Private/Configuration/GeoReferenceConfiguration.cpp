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

#include "Configuration/GeoReferenceConfiguration.h"

#include "Dom/JsonObject.h"

FString SGeoReferenceConfiguration::ConfigurationName = FString(TEXT("geo_reference"));
bool SGeoReferenceConfiguration::IsMandatory = true;

TSharedPtr<SGenericConfigurationBase> SGeoReferenceConfiguration::parse(TSharedPtr<FJsonObject> &jsonObj)
{
	const TSharedPtr<FJsonObject> *cfgObj = 0;
	if	(	jsonObj->TryGetObjectField(ConfigurationName, cfgObj)
		&&	cfgObj
		&&	(*cfgObj)->HasTypedField<EJson::String>("projected_crs")
		&&	(*cfgObj)->HasTypedField<EJson::Boolean>("shape_planet")
		&&	(*cfgObj)->HasTypedField<EJson::Array>("origin")
		)
	{
		TSharedPtr<SGeoReferenceConfiguration> geoRefCfg(new SGeoReferenceConfiguration);

		if (geoRefCfg)
		{
			geoRefCfg->ProjectedCRS = ((*cfgObj)->GetStringField("projected_crs"));
			geoRefCfg->IsPlanetShape = ((*cfgObj)->GetBoolField("shape_planet"));

			const TArray<TSharedPtr<FJsonValue>> arrayVals = (*cfgObj)->GetArrayField("origin");
			if(arrayVals.Num() == 3)
				geoRefCfg->Origin = FVector(arrayVals[0]->AsNumber(), arrayVals[1]->AsNumber(), arrayVals[2]->AsNumber());
		}

		return geoRefCfg;
	}

	return TSharedPtr<SGenericConfigurationBase>();
}
