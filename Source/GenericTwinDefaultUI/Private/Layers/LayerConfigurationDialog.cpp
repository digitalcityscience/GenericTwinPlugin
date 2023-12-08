// Fill out your copyright notice in the Description page of Project Settings.


#include "Layers/LayerConfigurationDialog.h"

#include "Layers/LayerBase.h"
#include "Layers/LayerConfigurationWidget.h"

void ULayerConfigurationDialog::Init(TSharedPtr<FJsonObject> Configuration, ULayerBase &Layer)
{
	Title = FString(TEXT("Configure Layer ")) + Layer.GetLayerMetaData().DisplayName.ToString();

	ULayerConfigurationWidget *configWidget = GetLayerConfigurationWidget();
	if(configWidget)
	{
		configWidget->Init(Configuration, Layer);
	}
}


ULayerConfigurationWidget* ULayerConfigurationDialog::GetLayerConfigurationWidget_Implementation()
{
	return 0;
}
