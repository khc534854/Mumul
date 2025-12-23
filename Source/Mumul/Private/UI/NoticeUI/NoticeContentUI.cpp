// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeUI/NoticeContentUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Network/WebSocketSubsystem.h"
#include "UI/NoticeUI/NoticeUI.h"

void UNoticeContentUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	NoticeConfirmBtn->OnClicked.AddDynamic(this, &UNoticeContentUI::OnConfirmClicked);
	
	WebSocketSystem = GetGameInstance()->GetSubsystem<UWebSocketSubsystem>();
}

void UNoticeContentUI::OnConfirmClicked()
{
	bIsConfirmed = true;
	UpdateConfirmButtonUI();
	
	// ️서버 저장
	if (WebSocketSystem && WebSocketSystem->IsConnected())
	{
		// TODO: SendStructMessage
		// FWSRequest_Query QueryReq;
		// QueryReq.sessionId = MyID; // 학습 챗봇은 sessionId = userId
		// QueryReq.userId = MyID;
		// QueryReq.query = Content;
		// QueryReq.grade = Grade;
		// WebSocketSystem->SendStructMessage(QueryReq);
	}
	else
	{
		bIsConfirmed = false;
		UpdateConfirmButtonUI();
	}
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
