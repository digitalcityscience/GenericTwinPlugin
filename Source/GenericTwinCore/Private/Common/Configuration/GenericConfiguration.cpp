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

#include "Common/Configuration/GenericConfiguration.h"
#include "Common/Configuration/GenericConfigurationBase.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY(LogGenericConfiguration);

GenericConfiguration::GenericConfiguration()
	:	m_JsonObject()
	,	m_Configurations()
{
}

bool GenericConfiguration::loadConfiguration(const FString &configFileName)
{
	bool res = false;
	FString jsonStr;
	if (FFileHelper::LoadFileToString(jsonStr, *configFileName))
	{
		UE_LOG(LogGenericConfiguration, Log, TEXT("Successfully opened configuration file %s"), *configFileName);
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonStr);

		if (FJsonSerializer::Deserialize(reader, m_JsonObject))
		{
			UE_LOG(LogGenericConfiguration, Log, TEXT("Successfully deserialized json config file"));

			m_ConfigFileName = configFileName;
			res = true;
		}
		else
			UE_LOG(LogGenericConfiguration, Error, TEXT("Not a valid json file"));
	}
	else
		UE_LOG(LogGenericConfiguration, Error, TEXT("Couldn't open configuration file %s"), *configFileName);

	return res;
}

TArray<FString> GenericConfiguration::GetStringList(const FString &Key)
{
	TArray<FString> list;

	if(m_JsonObject->HasTypedField<EJson::Array>(Key))
	{
		const TArray< TSharedPtr< FJsonValue > > &array = m_JsonObject->GetArrayField(Key);
		for(const TSharedPtr< FJsonValue > &val : array)
		{
			if(val->Type == EJson::String)
			{
				list.Add(val->AsString());
			}
		}
	}

	return list;
}
