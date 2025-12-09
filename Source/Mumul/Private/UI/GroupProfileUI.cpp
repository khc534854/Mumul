// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GroupProfileUI.h"

#include "Components/CheckBox.h"
#include "Components/TextBlock.h"

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
