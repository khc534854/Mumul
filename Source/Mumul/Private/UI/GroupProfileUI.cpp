// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GroupProfileUI.h"

#include "Components/CheckBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UGroupProfileUI::SetProfileIMG(UTexture2D* IMG)
{
	ProfileIMG->SetBrushFromTexture(IMG);
}

void UGroupProfileUI::SetPlayerName(FString Name)
{
	PlayerNameText->SetText(FText::FromString(Name));
}

FString UGroupProfileUI::GetPlayerName()
{
	return PlayerNameText->GetText().ToString();
}

bool UGroupProfileUI::GetCheckBoxState()
{
	return JoinedStateBox->IsChecked();
}
