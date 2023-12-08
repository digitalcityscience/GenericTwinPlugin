// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Common/WindowWidget.h"
#include "LayersListDialog.generated.h"


class ULayerManager;
class LayerBase;
class UDTVLayerConfigurationWidget;

USTRUCT(BlueprintType)
struct FLayerListItem
{ 
	GENERATED_BODY()
		
	UPROPERTY(BlueprintReadonly, Category = Default)
	int32	LayerId = 0;

	UPROPERTY(BlueprintReadonly, Category = Default)
	FText	DisplayName;

	UPROPERTY(BlueprintReadonly, Category = Default)
	FText	Description;

	UPROPERTY(BlueprintReadonly, Category = Default)
	bool	IsVisible = true;
};

/**
 * 
 */
UCLASS()
class GENERICTWINDEFAULTUI_API ULayersListDialog : public UWindowWidget
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLayerVisibilityChanged, int32, LayerId, bool, Visible);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClosed);

	void Init(ULayerManager* LayerMgr);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void SetLayers(const TArray<FLayerListItem> &LayerItems);

	UPROPERTY(BlueprintCallable)
	FOnLayerVisibilityChanged			OnLayerVisibilityChanged;

protected:

	UFUNCTION(BlueprintCallable, Category = Default)
	void OnAddLayer();

	UFUNCTION(BlueprintCallable, Category = Default)
	void OnConfigureLayer(int32 LayerId);

private:

	UFUNCTION()
	void OnChildDialogClosed();

	UFUNCTION()
	void UpdateLayerList();

	ULayerManager						*m_LayerManager = 0;

	UUserWidget							*m_ChildDialog = 0;
};
