// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LayerConfigurationWidget.generated.h"

class ULayerBase;

UENUM(BlueprintType)
enum class EDataType : uint8
{	
	UNDEFINED		UMETA(DisplayName = "Undefined"),
	STRING			UMETA(DisplayName = "String"),
	STRING_ARRAY	UMETA(DisplayName = "StringArray"),
	INT				UMETA(DisplayName = "Integer"),
	INT_ARRAY		UMETA(DisplayName = "IntegerArray"),
	NUMBER			UMETA(DisplayName = "Number"),
	NUMBER_ARRAY	UMETA(DisplayName = "NumberArray"),
	VECTOR2			UMETA(DisplayName = "Vector2"),
	VECTOR3			UMETA(DisplayName = "Vector3"),
	VECTOR4			UMETA(DisplayName = "Vector4"),
	COLOR			UMETA(DisplayName = "Color"),

};

/**
 * 
 */
UCLASS(BlueprintType)
class GENERICTWINDEFAULTUI_API ULayerConfigurationWidget : public UUserWidget
{
	GENERATED_BODY()

private:

	struct SParameter
	{
		FString		Key;
		FString		Value;
	};

public:

	void Init(TSharedPtr<FJsonObject> Configuration, const FString &LayerName);
	void Init(TSharedPtr<FJsonObject> Configuration, ULayerBase &Layer);

	TSharedPtr<FJsonObject> GetConfiguration() const;

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void Add(EDataType DataType, int32 ItemId, const FString &DisplayName, const FString &DefaultValue);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void AddString(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FString &Value);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void AddStringArray(int32 ItemId, const FString &DisplayName, bool IsRealtime, const TArray<FString> &Value);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void AddInt(int32 ItemId, const FString &DisplayName, bool IsRealtime, int32 Value);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void AddNumber(int32 ItemId, const FString &DisplayName, bool IsRealtime, double Value);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void AddVector2(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FVector2D &Value);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void AddVector3(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FVector &Value);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void AddVector4(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FVector4 &Value);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void AddColor(int32 ItemId, const FString &DisplayName, bool IsRealtime, const FLinearColor &Value);


	UPROPERTY(BlueprintReadonly, Category = Default)
	FString		Title;

	UFUNCTION(BlueprintCallable)
	void OnColorChanged(int32 ItemId, FLinearColor NewColor, bool UpdateLayerImmediately);

	UFUNCTION(BlueprintCallable)
	void OnStringChanged(int32 ItemId, const FString &NewString);


private:

	void ExtractParameters(TSharedPtr<FJsonObject> configurationJson, bool ValueRequired, bool RealtimeEnabled);

	void UpdateLayer(const FString &JsonStr);

	ULayerBase							*m_Layer = 0;

	TMap<int32, SParameter>				m_Parameters;

};
