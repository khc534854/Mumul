// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/PlayerNoticeComponent.h"

#include "Data/ObjectAndClassFinder.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Network/WebSocketSubsystem.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "UI/NoticeUI/NoticeUI.h"


UPlayerNoticeComponent::UPlayerNoticeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UPlayerNoticeComponent::BeginPlay()
{
	Super::BeginPlay();

	owner = Cast<ACuteAlienController>(GetOwner());
	if (owner)
		player = Cast<ACuteAlienPlayer>(owner->GetPawn());

	if (owner && owner->IsLocalController())
	{
		if (TSubclassOf<UNoticeUI> NoticeUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<
			UNoticeUI>("WBP_Notice"))
		{
			NoticeUI = CreateWidget<UNoticeUI>(owner, NoticeUIClass);
			if (NoticeUI)
			{
				NoticeUI->AddToViewport();
				NoticeUI->SetVisibility(ESlateVisibility::Visible);
			}
		}
		HttpSystem = owner->GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>();
		if (HttpSystem)
		{
			HttpSystem->OnLearningQuizResponse.AddDynamic(
				this, &UPlayerNoticeComponent::OnServerDispatchHistoryResponse);
		}
		WebSocketSystem = owner->GetGameInstance()->GetSubsystem<UWebSocketSubsystem>();
		if (WebSocketSystem)
		{
			WebSocketSystem->OnDispatchNotice.AddDynamic(this, &UPlayerNoticeComponent::OnNotice);
			WebSocketSystem->OnDispatchDM.AddDynamic(this, &UPlayerNoticeComponent::OnDirectMessage);
		}
	}
}


void UPlayerNoticeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerNoticeComponent::OnServerDispatchHistoryResponse(bool bSuccess, FString Message)
{
	if (bSuccess)
	{
		// 1. JSON 파싱 (Message에는 JSON 원본이 들어있음)
		FDispatchHistory DispatchHistory;

		if (FJsonObjectConverter::JsonObjectStringToUStruct(Message, &DispatchHistory, 0, 0))
		{
			// JSON Parsing LOG
			UE_LOG(LogTemp, Log, TEXT("[DispatchHistory] Parse Success. ItemCount = %d"), DispatchHistory.Items.Num());
			
			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("DispatchHistory 파싱 실패"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DispatchHistory Response 실패 : %s"), *Message);
	}
}

void UPlayerNoticeComponent::OnNotice(const FDispatchNoticePayload& Notice)
{
	if (NoticeUI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Notice] %s"), *Notice.noticeId);
		UE_LOG(LogTemp, Warning, TEXT("[Notice] %s"), *Notice.title);
		UE_LOG(LogTemp, Warning, TEXT("[Notice] %s"), *Notice.text);

		FNoticeData NoticeData;
		NoticeData.noticeId = *Notice.noticeId;
		NoticeData.title = *Notice.title;
		NoticeData.text = *Notice.text;
		NoticeData.CreatedAt = FDateTime::Now();

		NoticeUI->AddNotice(NoticeData);
	}
}

void UPlayerNoticeComponent::OnDirectMessage(const FDispatchDMPayload& DM)
{
	if (NoticeUI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DM] %s"), *DM.messageId);
		UE_LOG(LogTemp, Warning, TEXT("[DM] %s"), *DM.text);

		FDMData DMData;
		DMData.messageId = DM.messageId;
		DMData.text = DM.text;
		DMData.CreatedAt = FDateTime::Now();

		NoticeUI->AddDM(DMData);
	}
}
