// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuBar.generated.h"

/**
 * 
 */
UCLASS()
class GENERICTWINDEFAULTUI_API UMenuBar : public UUserWidget
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuItemSelected, int32, ItemId);


	UFUNCTION(BlueprintNativeEvent, Category=Default)
	int32 AddMenuItem(UTexture2D *MenuIcon);

	UPROPERTY(BlueprintCallable)
	FOnMenuItemSelected			OnMenuItemSelcted;

private:
	
};
