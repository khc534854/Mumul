// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/ChatMessageBlockUI.h"
#include "Components/TextBlock.h"


void UChatMessageBlockUI::SetContent(FString Content, FString Name, FString CurrentTime)
{
	TextContent->SetText(FText::FromString(Content));
	PlayerName->SetText(FText::FromString(Name));
	Time->SetText(FText::FromString(CurrentTime));
}
