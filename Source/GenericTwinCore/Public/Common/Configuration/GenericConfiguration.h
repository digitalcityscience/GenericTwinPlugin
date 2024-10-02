#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGenericConfiguration, Log, All);

class FJsonObject;
struct SGenericConfigurationBase;

class GenericConfiguration
{

public:

	GenericConfiguration();

	bool loadConfiguration(const FString &configFileName);

	template<class T>
	bool parse();

	template<class T>
	bool hasConfiguration() const;

	template<class T>
	const T& getConfiguration() const;

	FString getString(const FString &Key, const FString &DefaultValue = FString());

	const FString& GetConfigurationFileName() const;

	const TSharedPtr<FJsonObject>& GetRootObject() const;

private:

	template<class T>
	bool parse(TSharedPtr<FJsonObject>& jsonObject);

	TSharedPtr<FJsonObject>										m_JsonObject;

	TMap<FString, TSharedPtr<SGenericConfigurationBase>>		m_Configurations;

	FString														m_ConfigFileName;

};

template<class T>
inline bool GenericConfiguration::hasConfiguration() const
{
	return m_Configurations.Contains(T::ConfigurationName) && m_Configurations[T::ConfigurationName] != 0;
}


template<class T>
inline const T& GenericConfiguration::getConfiguration() const
{
	const TSharedPtr<SGenericConfigurationBase> ptr = m_Configurations[T::ConfigurationName];
	return *(static_cast<const T*>(ptr.operator->()));
}

template<class T>
inline bool GenericConfiguration::parse()
{
	TSharedPtr<SGenericConfigurationBase> config = T::parse(m_JsonObject);
	if (config)
	{
		m_Configurations.Add(T::ConfigurationName, config);
		return true;
	}
	return !T::IsMandatory;
}

template<class T>
inline bool GenericConfiguration::parse(TSharedPtr<FJsonObject> &jsonObject)
{
	SGenericConfigurationBase *config = T::parse(jsonObject);
	if(config)
	{
		m_Configurations.Add(T::ConfigurationName, config);
		return true;
	}
	return !T::IsMandatory;
}

inline FString GenericConfiguration::getString(const FString &Key, const FString &DefaultValue)
{
	return m_JsonObject->HasTypedField<EJson::String>(Key) ? m_JsonObject->GetStringField(Key) : DefaultValue;
}

inline const FString& GenericConfiguration::GetConfigurationFileName() const
{
	return m_ConfigFileName;
}

inline const TSharedPtr<FJsonObject>& GenericConfiguration::GetRootObject() const
{
	return m_JsonObject;
}
