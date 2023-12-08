
#include "DefaultUILayer.h"
#include "TwinWorldContext.h"
#include "Layers/LayerManager.h"

#include "MenuBar/MenuBar.h"
#include "Layers/LayersListDialog.h"

#include "UObject/SoftObjectPath.h"


const SLayerMetaData& UDefaultUILayer::GetLayerMetaData()
{
	static SLayerMetaData layerMetaData =	{	TEXT("DefaultUI")
											,	FText::FromString(TEXT("Default UI"))
											,	FText::FromString(TEXT("Default UI for handling layers, etc."))
											,	ELayerContributionType::UserInterface
											,	ELayerCreationType::StartupOnly
											};

	return layerMetaData;
}

ULayerBase* UDefaultUILayer::Create(FTwinWorldContext &TwinWorldCtx, const FJsonObject &Parameters)
{
	UDefaultUILayer* layer = NewObject<UDefaultUILayer>(TwinWorldCtx.World, FName(), RF_Standalone);

	if(layer)
	{
		layer->OnConstruction(GetLayerMetaData(), ELayerPriority::Medium);
	}

	return layer;
}

UDefaultUILayer::UDefaultUILayer()
	:	ULayerBase()
{
	
}

UDefaultUILayer::~UDefaultUILayer()
{
}

void UDefaultUILayer::InitializeLayer(FTwinWorldContext& TwinWorldCtx)
{
	FSoftClassPath widgetRef(TEXT("/Script/UMGEditor.WidgetBlueprint'/GenericTwinPlugin/DefaultUI/MenuBar/BPW_MenuBar.BPW_MenuBar_C'"));
	UClass* menuBarClass = widgetRef.TryLoadClass<UUserWidget>();
	m_MenuBar = menuBarClass ? Cast<UMenuBar>(CreateWidget(TwinWorldCtx.World, menuBarClass, FName())) : 0;
	if (m_MenuBar)
	{
		m_MenuBar->AddToViewport(10000);

		m_MenuBar->OnMenuItemSelcted.AddDynamic(this, &UDefaultUILayer::OnMenuItemSelectedImpl);

		widgetRef = FSoftClassPath(TEXT("/Script/UMGEditor.WidgetBlueprint'/GenericTwinPlugin/DefaultUI/Dialogs/BPW_LayersListDialog.BPW_LayersListDialog_C'"));
		m_LayersListDialogClass = widgetRef.TryLoadClass<UUserWidget>();

		if (m_LayersListDialogClass)
		{
			UTexture2D* layersIcon = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/GenericTwinPlugin/DefaultUI/Textures/T_IconLayers")));
			m_LayersMenuItemId = m_MenuBar->AddMenuItem(layersIcon);
		}
	}

	m_LayerManager = TwinWorldCtx.LayerManager;
}


void UDefaultUILayer::UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds)
{
}

void UDefaultUILayer::OnEndPlay(FTwinWorldContext &TwinWorldCtx)
{
}


void UDefaultUILayer::OnMenuItemSelectedImpl(int32 MenuItemId)
{
	if	(	MenuItemId == m_LayersMenuItemId
		&&	m_LayersListDialog == 0
		&&	m_LayersListDialogClass  != 0
		&&	m_LayerManager != 0
		)
	{
		OpenLayersList();
	}
}

void UDefaultUILayer::OpenLayersList()
{
	m_LayersListDialog = Cast<ULayersListDialog> (CreateWidget(GetWorld(), m_LayersListDialogClass, FName()));
	if(m_LayersListDialog)
	{
		m_LayersListDialog->AddToViewport(1000);
		m_LayersListDialog->OnLayerVisibilityChanged.AddDynamic(this, &UDefaultUILayer::OnLayerVisibilityChanged);
		m_LayersListDialog->OnClosed.AddDynamic(this, &UDefaultUILayer::OnLayerListClosed);

		m_LayersListDialog->Init(m_LayerManager);

		TArray<FLayerListItem> items;
		for(const auto &li : m_LayerManager->GetLayers())
		{
			ULayerBase *layer = li.Value;
			if (layer == this)
				continue;

			FLayerListItem item;
			item.LayerId = layer->GetLayerId();
			item.DisplayName = layer->GetLayerMetaData().DisplayName;
			item.Description = layer->GetLayerMetaData().Description;
			item.IsVisible = layer->GetVisibility();

			items.Add(item);
		}

		m_LayersListDialog->SetLayers(items);

	}
}

void UDefaultUILayer::OnLayerListClosed()
{
	if(m_LayersListDialog)
	{
		m_LayersListDialog->RemoveFromParent();
		m_LayersListDialog = 0;
	}
}

void UDefaultUILayer::OnLayerVisibilityChanged(int32 LayerId, bool Visible)
{
	if (m_LayerManager != 0)
	{
		m_LayerManager->SetLayerVisibility(LayerId, Visible);
	}
}
