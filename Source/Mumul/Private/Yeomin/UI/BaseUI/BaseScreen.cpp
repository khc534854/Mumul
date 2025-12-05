// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/BaseUI/BaseScreen.h"

#include "Components/Border.h"

void UBaseScreen::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(BackgroundColor);
	}
}