// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeUI/NoticeContentUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/NoticeUI/NoticeUI.h"

void UNoticeContentUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	NoticeConfirmBtn->OnClicked.AddDynamic(this, &UNoticeContentUI::OnConfirmClicked);
}

void UNoticeContentUI::OnConfirmClicked()
{
	bIsConfirmed = true;
	
	UpdateConfirmButtonUI();
	
	// ️서버 저장
	
}

void UNoticeContentUI::InitUI(const FNoticeViewData& Data)
{
	NoticeId = Data.Notice.NoticeId;
	CreatedAt = Data.Notice.CreatedAt;
	bIsConfirmed = Data.UserState.bConfirmed;

	NoticeContentText->SetText(FText::FromString(Data.Notice.Content));
	UpdateConfirmButtonUI();
}

void UNoticeContentUI::UpdateConfirmButtonUI()
{
	if (bIsConfirmed)
	{
		NoticeConfirmText->SetText(FText::FromString(TEXT("확인됨")));
		NoticeConfirmBtn->SetIsEnabled(false);
		
		// 확인됨 색상
		FSlateColor ConfirmColor(FLinearColor::Green);
		NoticeConfirmText->SetColorAndOpacity(ConfirmColor);
	}
	else
	{
		NoticeConfirmText->SetText(FText::FromString(TEXT("확인")));
		NoticeConfirmBtn->SetIsEnabled(true);

		// 확인전 색상
		FSlateColor NeedConfirmColor(FLinearColor::Red);
		NoticeConfirmText->SetColorAndOpacity(NeedConfirmColor);
	}
}
