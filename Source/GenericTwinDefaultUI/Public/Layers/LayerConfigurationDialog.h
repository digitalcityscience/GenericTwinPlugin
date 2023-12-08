// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Common/WindowWidget.h"
#include "LayerConfigurationDialog.generated.h"

class ULayerBase;
class ULayerConfigurationWidget;

/**
 * 
 */
UCLASS()
class GENERICTWINDEFAULTUI_API ULayerConfigurationDialog : public UWindowWidget
{
	GENERATED_BODY()

public:

	void Init(TSharedPtr<FJsonObject> Configuration, ULayerBase &Layer);

protected:

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	ULayerConfigurationWidget* GetLayerConfigurationWidget();

private:

};
