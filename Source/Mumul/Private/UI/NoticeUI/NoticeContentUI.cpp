// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeUI/NoticeContentUI.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Network/WebSocketSubsystem.h"
#include "Player/CuteAlienController.h"
#include "Player/MumulPlayerState.h"
#include "Player/Component/PlayerNoticeComponent.h"
#include "UI/PlayerUI.h"
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
			AMumulPlayerState* PS = Cast<AMumulPlayerState>(GetOwningPlayerState());
			WebSocketSystem->SendDispatchAck(TEXT("notice"), ContentID, PS->PS_UserIndex);
		}
	}
	
	if (Cast<ACuteAlienController>(GetOwningPlayer())->NoticeComp->NoticeUI->IsAllConfirmed())
	{
		Cast<ACuteAlienController>(GetOwningPlayer())->PlayerUI->NewNoticeBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UNoticeContentUI::InitUI(const FDispatchPayloadBase& Data)
{
	ContentID = Data.MessageId;
	CreatedAt = Data.CreatedAt;
	bIsConfirmed = Data.IsConfirmed;

	if (Data.NeedConfirmation)
	{
		FString Content;
		if (Data.Title.IsEmpty())
		{
			Content = FString::Printf(TEXT("[개인메시지] \n%s"), *Data.Text);
		}
		else
		{
			Content = FString::Printf(TEXT("[%s] \n%s"), *Data.Title, *Data.Text);
		}

		NoticeContentText->SetText(FText::FromString(Content));
		UpdateConfirmButtonUI();
	}
	else
	{
		NoticeContentText->SetText(FText::FromString(Data.Text));
		NoticeConfirmBtn->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UNoticeContentUI::UpdateConfirmButtonUI()
{
	if (bIsConfirmed)
	{
		NoticeConfirmText->SetText(FText::FromString(TEXT("확인됨")));
		NoticeConfirmBtn->SetIsEnabled(false);

		// 확인됨 색상
		// FSlateColor ConfirmColor(FLinearColor::Green);
		// NoticeConfirmText->SetColorAndOpacity(ConfirmColor);
	}
	else
	{
		NoticeConfirmText->SetText(FText::FromString(TEXT("확인")));
		NoticeConfirmBtn->SetIsEnabled(true);

		// 확인전 색상
		// FSlateColor NeedConfirmColor(FLinearColor::Red);
		// NoticeConfirmText->SetColorAndOpacity(NeedConfirmColor);
	}
}

void UNoticeContentUI::SetTimeStampText(const FDateTime& Time)
{
	// UTC → KST
	const FDateTime Kst = Time + FTimespan(9, 0, 0);
	
	UE_LOG(LogTemp, Warning, TEXT("Dispatch -> SetTimeStampText()"));
	UE_LOG(LogTemp, Warning, TEXT("Raw Time (Server): %s"), *Time.ToString());
	UE_LOG(LogTemp, Warning, TEXT("After +9h: %s"), *Kst.ToString());
	UE_LOG(LogTemp, Warning, TEXT("Local Now: %s"), *FDateTime::Now().ToString());
	UE_LOG(LogTemp, Warning, TEXT("UTC Now: %s"), *FDateTime::UtcNow().ToString());

	// 날짜
	const FString DatePart = FString::Printf(
		TEXT("%04d-%02d-%02d"),
		Kst.GetYear(),
		Kst.GetMonth(),
		Kst.GetDay()
	);

	// 시간 (12시간제)
	int32 Hour = Kst.GetHour();
	const TCHAR* AmPm = Hour < 12 ? TEXT("AM") : TEXT("PM");

	Hour %= 12;
	if (Hour == 0) Hour = 12;

	const FString TimePart = FString::Printf(
		TEXT("%s %02d:%02d"),
		AmPm,
		Hour,
		Kst.GetMinute()
	);

	// 최종 텍스트
	const FString FinalText = FString::Printf(
		TEXT("%s %s"),
		*DatePart,
		*TimePart
	);

	TimeStampText->SetText(FText::FromString(FinalText));
}
