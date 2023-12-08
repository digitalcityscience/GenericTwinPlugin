// Fill out your copyright notice in the Description page of Project Settings.


#include "Layers/LayersListDialog.h"
// #include "Layers/DTVLayerConfigurationWindow.h"
// #include "Layers/DTVAddLayerWindow.h"

void ULayersListDialog::Init(ULayerManager *LayerMgr)
{
	m_LayerManager = LayerMgr;
	if(m_LayerManager)
	{
		m_LayerManager->OnLayerListUpdated.AddDynamic(this, &ULayersListDialog::UpdateLayerList);
		UpdateLayerList();
	}
}

void ULayersListDialog::SetLayers_Implementation(const TArray<FLayerListItem> &LayerItems)
{
}

void ULayersListDialog::OnConfigureLayer(int32 LayerId)
{
	// if	(	m_LayerManager != 0
	// 	&&	m_ChildDialog == 0
	// 	)
	// {
	// 	LayerBase *layerToConfigure = m_LayerManager->GetLayer(LayerId);
	// 	if (layerToConfigure)
	// 	{
	// 		TSharedPtr<FJsonObject> jsonObject = layerToConfigure->GetRuntimeConfiguration();
	// 		if	(	jsonObject
	// 			&&	jsonObject->HasTypedField<EJson::Array>(TEXT("configuration"))
	// 			)
	// 		{
	// 			FSoftClassPath widgetRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/DigitalTwinViewerDefaultUI/Widgets/Layers/BPW_DTVLayerConfigurationWindow.BPW_DTVLayerConfigurationWindow_C'"));
	// 			UClass* widgetClass = widgetRef.TryLoadClass<UUserWidget>();
	// 			UDTVLayerConfigurationWindow *layerConfigWindow = widgetClass ? Cast<UDTVLayerConfigurationWindow>(CreateWidget(GetWorld(), widgetClass, FName())) : 0;
	// 			if (layerConfigWindow)
	// 			{
	// 				m_ChildDialog = layerConfigWindow;
	// 				layerConfigWindow->OnClosed.AddDynamic(this, &ULayersListDialog::OnChildDialogClosed);
			
	// 				layerConfigWindow->AddToViewport(1100);
	// 				layerConfigWindow->Init(jsonObject, *layerToConfigure);

	// 				SetIsBlocked(true);

	// 			}
	// 		}
	// 	}
		
	// }
}


void ULayersListDialog::OnAddLayer()
{
	// if	(	m_LayerManager != 0
	// 	&&	m_ChildDialog == 0
	// 	)
	// {
	// 	FSoftClassPath widgetRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/DigitalTwinViewerDefaultUI/Widgets/Layers/BPW_DTVAddLayerWindow.BPW_DTVAddLayerWindow_C'"));
	// 	UClass* widgetClass = widgetRef.TryLoadClass<UUserWidget>();
	// 	UDTVAddLayerWindow *addLayerWindow = widgetClass ? Cast<UDTVAddLayerWindow>(CreateWidget(GetWorld(), widgetClass, FName())) : 0;
	// 	if (addLayerWindow)
	// 	{
	// 		m_ChildDialog = addLayerWindow;
	// 		addLayerWindow->OnClosed.AddDynamic(this, &ULayersListDialog::OnChildDialogClosed);
	
	// 		addLayerWindow->AddToViewport(1100);
	// 		addLayerWindow->Init(*m_LayerManager);

	// 		SetIsBlocked(true);
	// 	}
	// }
}

void ULayersListDialog::OnChildDialogClosed()
{
	if(m_ChildDialog)
	{
		m_ChildDialog->RemoveFromParent();
		m_ChildDialog = 0;
		SetIsBlocked(false);
	}
}

void ULayersListDialog::UpdateLayerList()
{
	TArray<FLayerListItem> items;
	for(const auto &li : m_LayerManager->GetLayers())
	{
		ULayerBase *layer = li.Value;
		const ELayerContributionType layerType = layer->GetLayerContributionType();
		if (layerType == ELayerContributionType::UserInterface)
			continue;

		FLayerListItem item;
		item.LayerId = layer->GetLayerId();
		item.DisplayName = layer->GetLayerMetaData().DisplayName;
		item.Description = layer->GetLayerMetaData().Description;
		item.IsVisible = layer->GetVisibility();

		items.Add(item);
	}

	SetLayers(items);
}
