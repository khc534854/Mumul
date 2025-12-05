// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/BaseUI/BaseTextBox.h"

#include "Components/MultiLineEditableTextBox.h"

void UBaseTextBox::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (BaseTextBox)
	{
		FSlateFontInfo Font = BaseFont;
		Font.Size = FontSize;
		BaseTextBox->WidgetStyle.SetFont(Font);
		
		BaseTextBox->SetText(Text);
		BaseTextBox->SetHintText(HintText);
	}
}
