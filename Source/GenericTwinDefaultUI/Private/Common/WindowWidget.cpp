// Fill out your copyright notice in the Description page of Project Settings.


#include "Common/WindowWidget.h"

void UWindowWidget::SetIsBlocked_Implementation(bool _IsBlocked)
{
	if(_IsBlocked != IsBlocked)
	{
		IsBlocked = _IsBlocked;
		OnIsBlockedChanged();
	}
}

void UWindowWidget::OnIsBlockedChanged()
{
}
