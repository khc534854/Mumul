// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/PlayerNoticeComponent.h"

#include "Data/ObjectAndClassFinder.h"
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

void UPlayerNoticeComponent::OnNotice(const FDispatchNoticePayload& Notice)
{
	FNoticeViewData NoticeData;
    
	FString DisplayContent = FString::Printf(TEXT("[%s] %s\n%s"), *Notice.urgency, *Notice.title, *Notice.text);
	NoticeData.Notice.Content = DisplayContent;
    
	NoticeData.Notice.CreatedAt = FDateTime::Now();
	NoticeData.UserState.bConfirmed = false;

	if (NoticeUI)
	{
		NoticeUI->AddNotice(NoticeData);
        
		if (WebSocketSystem)
		{
			WebSocketSystem->SendDispatchAck(TEXT("notice"), Notice.noticeId);
		}
	}
}

void UPlayerNoticeComponent::OnDirectMessage(const FDispatchDMPayload& DM)
{
	FNoticeViewData NoticeData;
	NoticeData.Notice.Content = DM.text;
	NoticeData.Notice.CreatedAt = FDateTime::Now();
	NoticeData.UserState.bConfirmed = false;

	if (NoticeUI)
	{
		NoticeUI->AddDM(NoticeData);
        
		// (선택) 수신 확인 ACK 자동 전송
		if (WebSocketSystem)
		{
			WebSocketSystem->SendDispatchAck(TEXT("dm"), DM.messageId);
		}
	}
}