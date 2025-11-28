// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/GroupIconUI.h"

#include "Components/Button.h"

void UGroupIconUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	GroupIconBtn->OnPressed.AddDynamic(this, &UGroupIconUI::DisplayGroupChat);
}

void UGroupIconUI::DisplayGroupChat()
{
	
}
