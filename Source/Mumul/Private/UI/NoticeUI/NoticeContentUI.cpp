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
	if (WebSocketSystem)
	{
		WebSocketSystem->OnAckPongReceived.AddDynamic(this, &UNoticeContentUI::OnAckResponse);
	}
}

void UNoticeContentUI::OnAckResponse(const FDispatchAckPongPayload& AckPong)
{
	UE_LOG(LogTemp, Warning, TEXT("[OnAckResponse] : %s"), AckPong.ok ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Warning, TEXT("[OnAckResponse] : %s"), *AckPong.serverAt)
	
	// if (AckPong.ok)
	// 	return;
	//
	// bIsConfirmed = false;
	// UpdateConfirmButtonUI();
}

void UNoticeContentUI::OnConfirmClicked()
{
	bIsConfirmed = true;
	UpdateConfirmButtonUI();

	// ️서버 저장
	if (WebSocketSystem && WebSocketSystem->IsConnected())
	{
		if (bIsNotice)
		{
			WebSocketSystem->SendDispatchAck(TEXT("notice"), ContentID);
		}
		else
		{
			WebSocketSystem->SendDispatchAck(TEXT("dm"), ContentID);
		}
	}
}

void UNoticeContentUI::InitUI(const FNoticeData& Data)
{
	ContentID = Data.noticeId;
	CreatedAt = Data.CreatedAt;
	bIsConfirmed = Data.bConfirmed;

	FString Content = FString::Printf(TEXT("[%s] \n%s"), *Data.title, *Data.text);

	NoticeContentText->SetText(FText::FromString(Content));
	UpdateConfirmButtonUI();
}

void UNoticeContentUI::InitUI(const FDMData& Data)
{
	ContentID = Data.messageId;
	CreatedAt = Data.CreatedAt;
	bIsConfirmed = Data.bConfirmed;
	bIsNotice = false;

	NoticeContentText->SetText(FText::FromString(Data.text));
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
