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
			WebSocketSystem->OnNoticeReceived.AddDynamic(this, &UPlayerNoticeComponent::OnNotice);
			WebSocketSystem->OnDirectMessageReceived.AddDynamic(this, &UPlayerNoticeComponent::OnDirectMessage);
		}
	}
}


void UPlayerNoticeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerNoticeComponent::OnNotice(const FString& NoticeMessage)
{
	// TODO: AddNotice Struct
	// Test Add Notice
	FNoticeViewData NoticeData;
	NoticeData.Notice.Content = FString(TEXT("Test Notice!"));
	NoticeData.Notice.CreatedAt = FDateTime::Now();
	NoticeData.UserState.bConfirmed = false;
	NoticeUI->AddNotice(NoticeData);
}

void UPlayerNoticeComponent::OnDirectMessage(const FString& NoticeMessage)
{
	// TODO: AddDM Struct
	// Test Add DM
	FNoticeViewData NoticeData;
	NoticeData.Notice.Content = FString(TEXT("Test DM!"));
	NoticeData.Notice.CreatedAt = FDateTime::Now();
	NoticeData.UserState.bConfirmed = false;
	NoticeUI->AddDM(NoticeData);
}
