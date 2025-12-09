// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ChatMessageBlockUI.h"
#include "Components/TextBlock.h"


void UChatMessageBlockUI::SetContent(const FString& CurrentTime, const FString& Name, const FString& Content) const
{
	TextContent->SetText(FText::FromString(Content));
	PlayerName->SetText(FText::FromString(Name));
	Time->SetText(FText::FromString(CurrentTime));
}
