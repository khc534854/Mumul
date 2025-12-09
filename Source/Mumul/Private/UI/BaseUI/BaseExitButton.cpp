// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/BaseUI/BaseExitButton.h"

#include "Components/Button.h"

void UBaseExitButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (BaseExitButton && ButtonImage)
	{
		FSlateBrush Brush;
		Brush.SetResourceObject(ButtonImage);
		Brush.ImageSize = FVector2D(ImageSize, ImageSize);

		FButtonStyle BtnStyle;
		BtnStyle.Normal = Brush;
		BtnStyle.Hovered = Brush;
		BtnStyle.Pressed = Brush;

		BaseExitButton->SetStyle(BtnStyle);
	}
}
