// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WindowWidget.generated.h"

/**
 * 
 */
UCLASS()
class GENERICTWINDEFAULTUI_API UWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClosed);

	UFUNCTION(BlueprintNativeEvent, Category=Default)
	void SetIsBlocked(bool _IsBlocked);

	UPROPERTY(BlueprintCallable)
	FOnClosed							OnClosed;

protected:

	virtual void OnIsBlockedChanged();

	UPROPERTY(BlueprintReadonly, Category = Default)
	FString		Title;

	UPROPERTY(BlueprintReadonly, Category = Default)
	bool IsBlocked = false;

};
