#pragma once

#include "Layers/LayerBase.h"
#include "Layers/LayerTypes.h"

#include "DefaultUILayer.generated.h" 


class FJsonObject;
class ULayersListDialog;
class UMenuBar;
class ULayerManager;

UCLASS()
class UDefaultUILayer	:	public ULayerBase
{
	GENERATED_BODY()

public:

	static const SLayerMetaData& GetLayerMetaData();
	static ULayerBase* Create(FTwinWorldContext &TwinWorldCtx, const TSharedPtr<FJsonObject> &Parameters);

	virtual ~UDefaultUILayer();

	virtual void InitializeLayer(FTwinWorldContext &TwinWorldCtx) override;

	virtual void UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds) override;

	virtual void OnEndPlay(FTwinWorldContext& TwinWorldCtx) override;


private:

	UDefaultUILayer();

	UFUNCTION()
	void OnMenuItemSelectedImpl(int32 MenuItemId);

	UFUNCTION()
	void OnLayerVisibilityChanged(int32 LayerId, bool Visible);

	UFUNCTION()
	void OnLayerListClosed();

	void OpenLayersList();


	ULayerManager							*m_LayerManager = 0;

	UMenuBar								*m_MenuBar = 0;

	UClass									*m_LayersListDialogClass = 0;
	ULayersListDialog						*m_LayersListDialog = 0;
	int32									m_LayersMenuItemId = 0;

};
