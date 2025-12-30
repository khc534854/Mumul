// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeUI/DispatchPopUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/CuteAlienController.h"
#include "Player/Component/PlayerNoticeComponent.h"
#include "UI/NoticeUI/NoticeUI.h"


void UDispatchPopUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	PC = Cast<ACuteAlienController>(GetOwningPlayer());
	
	DispatchBtn->OnClicked.AddDynamic(this, &UDispatchPopUI::OnClickPopUp);
}

void UDispatchPopUI::OnClickPopUp()
{
	if (PC->NoticeComp->NoticeUI->bIsNoticeVisible == false)
	{
		PC->NoticeComp->NoticeUI->OnToggleNoticeVisibility();
	}
	DispatchBtn->OnClicked.RemoveDynamic(this, &UDispatchPopUI::OnClickPopUp);
}

FString UDispatchPopUI::MakePreviewText(const FString& OriginalText, int32 MaxLength)
{
	if (OriginalText.Len() <= MaxLength)
	{
		return OriginalText;
	}
	return OriginalText.Left(MaxLength) + TEXT("…");
}

void UDispatchPopUI::SetDispatchText(const FString& Text)
{
	const int32 PreviewLength = 70; // 원하는 미리보기 길이
	FString PreviewText = MakePreviewText(Text, PreviewLength);
	
	DispatchText->SetText(FText::FromString(PreviewText));
}

