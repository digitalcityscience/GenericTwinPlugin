#pragma once

#include "CoreMinimal.h"

class FJsonObject;

class GENERICTWINCORE_API ConfigurationJsonBuilder
{

public:

	void Begin();
	TSharedPtr<FJsonObject> Finalize();

	void Add(const FString &Key, const FString &Type, const FString &DisplayName, bool IsRealtime);
	void AddLiteralValue(const FString &Key, const FString &Type, const FString &DisplayName, bool IsRealtime, const FString &Value);
	void AddStringValue(const FString &Key, const FString &Type, const FString &DisplayName, bool IsRealtime, const FString &Value);
	void AddStringOption(const FString &Key, const FString &DisplayName, bool IsRealtime, const TArray<FString> &Options);

private:

	FString					m_JsonString;

	bool					m_addComma = false;

};
