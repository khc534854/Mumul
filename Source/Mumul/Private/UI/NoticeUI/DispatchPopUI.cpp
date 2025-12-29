// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeUI/DispatchPopUI.h"

#include "Components/TextBlock.h"

void UDispatchPopUI::SetDispatchText(const FString& Text)
{
	DispatchText->SetText(FText::FromString(Text));
}
