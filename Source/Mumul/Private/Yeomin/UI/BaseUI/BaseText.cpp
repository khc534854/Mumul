// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/BaseUI/BaseText.h"

#include "Components/TextBlock.h"

void UBaseText::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	if (BaseText)
	{
		FSlateFontInfo Font = BaseFont;
		Font.Size = FontSize;

		BaseText->SetFont(Font);
		BaseText->SetText(Text);
	}
}
