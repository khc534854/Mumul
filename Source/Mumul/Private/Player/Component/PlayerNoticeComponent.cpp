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
	UE_LOG(LogTemp, Warning, TEXT("[Notice] %s"), *Notice.noticeId);
	UE_LOG(LogTemp, Warning, TEXT("[Notice] %s"), *Notice.title);
	UE_LOG(LogTemp, Warning, TEXT("[Notice] %s"), *Notice.text);
	
	FNoticeData NoticeData;
	NoticeData.noticeId = *Notice.noticeId;
	NoticeData.title = *Notice.title;
	NoticeData.text = *Notice.text;
	NoticeData.CreatedAt = FDateTime::Now();

	if (NoticeUI)
	{
		NoticeUI->AddNotice(NoticeData);
	}
}

void UPlayerNoticeComponent::OnDirectMessage(const FDispatchDMPayload& DM)
{
	UE_LOG(LogTemp, Warning, TEXT("[DM] %s"), *DM.messageId);
	UE_LOG(LogTemp, Warning, TEXT("[DM] %s"), *DM.text);
	
	FDMData DMData;
	DMData.messageId = DM.messageId;
	DMData.text = DM.text;
	DMData.CreatedAt = FDateTime::Now();

	if (NoticeUI)
	{
		NoticeUI->AddDM(DMData);
	}
}