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

#include "Common/Configuration/ConfigurationJsonBuilder.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"


void ConfigurationJsonBuilder::Begin()
{
	m_JsonString = TEXT("{\"configuration\":[");
	m_addComma = false;
}

TSharedPtr<FJsonObject> ConfigurationJsonBuilder::Finalize()
{
	m_JsonString += TEXT("]}");

	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(m_JsonString);
	TSharedPtr<FJsonObject> jsonObject;
	FJsonSerializer::Deserialize(reader, jsonObject);
	return jsonObject;
}

void ConfigurationJsonBuilder::Add(const FString &Key, const FString &Type, const FString &DisplayName, bool IsRealtime)
{
	if(m_addComma)
		m_JsonString += TEXT(",");
	m_addComma = true;
	const FString jsonFormatStr = FString(TEXT("{\"key\":\"{0}\",\"type\":\"{1}\",\"display_name\":\"{2}\",\"is_realtime\":{3}}"));
	m_JsonString += FString::Format(*jsonFormatStr, {Key, Type, DisplayName, IsRealtime ? TEXT("true") : TEXT("false") });
}

void ConfigurationJsonBuilder::AddLiteralValue(const FString &Key, const FString &Type, const FString &DisplayName, bool IsRealtime, const FString &Value)
{
	if(m_addComma)
		m_JsonString += TEXT(",");
	m_addComma = true;
	const FString jsonFormatStr = FString(TEXT("{\"key\":\"{0}\",\"type\":\"{1}\",\"display_name\":\"{2}\",\"is_realtime\":{3},\"value\":{4}}"));
	m_JsonString += FString::Format(*jsonFormatStr, {Key, Type, DisplayName, IsRealtime ? TEXT("true") : TEXT("false"), Value });
}

void ConfigurationJsonBuilder::AddStringValue(const FString &Key, const FString &Type, const FString &DisplayName, bool IsRealtime, const FString &Value)
{
	if(m_addComma)
		m_JsonString += TEXT(",");
	m_addComma = true;
	const FString jsonFormatStr = FString(TEXT("{\"key\":\"{0}\",\"type\":\"{1}\",\"display_name\":\"{2}\",\"is_realtime\":{3},\"value\":\"{4}\"}"));
	m_JsonString += FString::Format(*jsonFormatStr, {Key, Type, DisplayName, IsRealtime ? TEXT("true") : TEXT("false"), Value });
}

void ConfigurationJsonBuilder::AddStringOption(const FString &Key, const FString &DisplayName, bool IsRealtime, const TArray<FString> &Options)
{
	if(m_addComma)
		m_JsonString += TEXT(",");
	m_addComma = true;
	FString value = TEXT("[");
	bool addComma = false;
	for(auto &option : Options)
	{
		if(addComma)
			value += TEXT(",");
		value += TEXT("\"") + option + TEXT("\"");
		addComma = true;
	}
	value += TEXT("]");
	const FString jsonFormatStr = FString(TEXT("{\"key\":\"{0}\",\"type\":\"string_option\",\"display_name\":\"{1}\",\"is_realtime\":{2},\"value\":{3}}"));
	m_JsonString += FString::Format(*jsonFormatStr, {Key, DisplayName, IsRealtime ? TEXT("true") : TEXT("false"), value });
}
