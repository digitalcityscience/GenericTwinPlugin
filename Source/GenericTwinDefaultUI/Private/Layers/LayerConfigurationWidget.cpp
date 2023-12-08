// Fill out your copyright notice in the Description page of Project Settings.


#include "Layers/LayerConfigurationWidget.h"

#include "Layers/LayerBase.h"

void ULayerConfigurationWidget::Init(TSharedPtr<FJsonObject> Configuration, const FString &LayerName)
{
	Title = FString(TEXT("Configure Layer ")) + LayerName;

	m_Layer = 0;
	ExtractParameters(Configuration, false, false);
}

void ULayerConfigurationWidget::Init(TSharedPtr<FJsonObject> Configuration, ULayerBase &Layer)
{
	Title = FString(TEXT("Configure Layer ")) + Layer.GetLayerMetaData().DisplayName.ToString();

	m_Layer = &Layer;
	ExtractParameters(Configuration, true, true);
}

TSharedPtr<FJsonObject> ULayerConfigurationWidget::GetConfiguration() const
{
	// wrap all updated parameters into a JSON object
	FString jsonStr = TEXT("{");

	bool addComma = false;
	for(const auto &p : m_Parameters)
	{
		if(addComma)
			jsonStr += TEXT(",");
		addComma = true;
		jsonStr += FString::Format(TEXT("\"{0}\":{1}"), {p.Value.Key, p.Value.Value});
	}

	jsonStr += TEXT("}");

	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonStr);
	TSharedPtr<FJsonObject> jsonObject;
	FJsonSerializer::Deserialize(reader, jsonObject);

	return jsonObject;
}

void ULayerConfigurationWidget::ExtractParameters(TSharedPtr<FJsonObject> configurationJson, bool ValueRequired, bool RealtimeEnabled)
{
	const TArray < TSharedPtr < FJsonValue > > *items = 0;
	if	(	configurationJson
		&&	configurationJson->TryGetArrayField(TEXT("configuration"), items)
		&&	items
		)
	{
		int32 nextItemId = 1;
		for(const auto &item : (*items))
		{
			const TSharedPtr < FJsonObject > *itemObject = 0;
			if	(	item->TryGetObject(itemObject)
				&&	itemObject
				&&	(*itemObject)->HasTypedField<EJson::String>(TEXT("key"))
				&&	(*itemObject)->HasTypedField<EJson::String>(TEXT("type"))
				&&	(*itemObject)->HasTypedField<EJson::String>(TEXT("display_name"))
				&&	(ValueRequired == false || (*itemObject)->HasField("value"))
				)
			{
				FString key = (*itemObject)->GetStringField(TEXT("key"));
				FString type = (*itemObject)->GetStringField(TEXT("type"));
				FString displayName = (*itemObject)->GetStringField(TEXT("display_name"));

				FString valueString;

				bool isRealtime = false;
				if(RealtimeEnabled)
					(*itemObject)->TryGetBoolField(TEXT("is_realtime"), isRealtime);

				if(type ==  TEXT("string"))
				{
					FString value;
					(*itemObject)->TryGetStringField(TEXT("value"), value);
					AddString(nextItemId, displayName, isRealtime, value);

					valueString = TEXT("\"") + value + TEXT("\"");
				}
				else if(type ==  TEXT("string_array"))
				{
					TArray<FString> value;
					valueString = TEXT("[");
					if((*itemObject)->HasTypedField<EJson::Array>("value"))
					{
						const TArray<TSharedPtr<FJsonValue>> arrayVals = (*itemObject)->GetArrayField("value");
						for(const auto &val : arrayVals)
						{
							value.Add(val->AsString());
						}
					}
					valueString += TEXT("]");

					AddStringArray(nextItemId, displayName, isRealtime, value);
				}
				else if(type ==  TEXT("int"))
				{
				}
				else if(type ==  TEXT("number"))
				{
				}
				else if(type ==  TEXT("vector2"))
				{
				}
				else if(type ==  TEXT("vector3"))
				{
				}
				else if(type ==  TEXT("vector4"))
				{
				}
				else if(type ==  TEXT("color"))
				{
					FLinearColor color(0.0, 0.0, 0.0, 1.0);
					if((*itemObject)->HasTypedField<EJson::Array>("value"))
					{
						const TArray<TSharedPtr<FJsonValue>> arrayVals = (*itemObject)->GetArrayField("value");
						if(arrayVals.Num() == 3)
						{
							color = FLinearColor(arrayVals[0]->AsNumber(), arrayVals[1]->AsNumber(), arrayVals[2]->AsNumber(), 1.0);
						}
						else if(arrayVals.Num() == 4)
						{
							color = FLinearColor(arrayVals[0]->AsNumber(), arrayVals[1]->AsNumber(), arrayVals[2]->AsNumber(), arrayVals[3]->AsNumber());
						}
					}
					AddColor(nextItemId, displayName, isRealtime, color);
					valueString = FString::Format(TEXT("[{0},{1},{2},{3}]"), { color.R, color.G, color.B, color.A });
				}

				m_Parameters.Add(nextItemId, {key, valueString});

				++nextItemId;
			}
		}
	}
}


void ULayerConfigurationWidget::OnColorChanged(int32 ItemId, FLinearColor NewColor, bool UpdateLayerImmediately)
{
	if (m_Parameters.Contains(ItemId))
	{
		const FString valueStr = FString::Format(TEXT("[{0},{1},{2},1.0]"), { NewColor.R, NewColor.G, NewColor.B });
		m_Parameters[ItemId].Value = valueStr;
		if(UpdateLayerImmediately)
		{
			const FString jsonStr = FString::Format(TEXT("{\"{0}\":{1}}"), {m_Parameters[ItemId].Key, valueStr });
			UpdateLayer(jsonStr);
		}
	}
}


void ULayerConfigurationWidget::OnStringChanged(int32 ItemId, const FString &ChangedString)
{
	if (m_Parameters.Contains(ItemId))
	{
		m_Parameters[ItemId].Value = TEXT("\"") + ChangedString + TEXT("\"");
	}
}


void ULayerConfigurationWidget::UpdateLayer(const FString &JsonStr)
{
	if(m_Layer)
	{
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(JsonStr);
		TSharedPtr<FJsonObject> jsonObject;
		FJsonSerializer::Deserialize(reader, jsonObject);

		if(jsonObject)
			m_Layer->UpdateConfiguration(jsonObject);
	}

}

void ULayerConfigurationWidget::Add_Implementation(EDataType DataType, int32 ItemId, const FString &DisplayName, const FString &DefaultValue)
{
}

void ULayerConfigurationWidget::AddString_Implementation(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FString &Value)
{
}

void ULayerConfigurationWidget::AddStringArray_Implementation(int32 ItemId, const FString &DisplayName, bool IsRealtime, const TArray<FString> &Value)
{
}

void ULayerConfigurationWidget::AddInt_Implementation(int32 ItemId, const FString &DisplayName, bool IsRealtime, int32 Value)
{
}

void ULayerConfigurationWidget::AddNumber_Implementation(int32 ItemId, const FString &DisplayName, bool IsRealtime, double Value)
{
}

void ULayerConfigurationWidget::AddVector2_Implementation(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FVector2D &Value)
{
}

void ULayerConfigurationWidget::AddVector3_Implementation(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FVector &Value)
{
}

void ULayerConfigurationWidget::AddVector4_Implementation(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FVector4 &Value)
{
}

void ULayerConfigurationWidget::AddColor_Implementation(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FLinearColor &Value)
{
}

