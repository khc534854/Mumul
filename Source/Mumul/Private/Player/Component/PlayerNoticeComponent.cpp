// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/PlayerNoticeComponent.h"

#include "Base/MumulGameState.h"
#include "Components/Border.h"
#include "Data/AudioManager.h"
#include "Data/ObjectAndClassFinder.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Network/WebSocketSubsystem.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "Player/MumulPlayerState.h"
#include "UI/PlayerUI.h"
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
			HttpSystem->OnDispatchHistoryResponse.AddDynamic(
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
			for (int32 Index = 0; Index < DispatchHistory.Items.Num(); ++Index)
			{
				const FDispatchItem& Item = DispatchHistory.Items[Index];
				const FDispatchPayloadBase& Payload = Item.Payload;
				UE_LOG(LogTemp, Log,
				       TEXT("[Item %d] Domain=%s Event=%s"),
				       Index,
				       *Item.Domain,
				       *Item.Event
				);
				UE_LOG(LogTemp, Log,
				       TEXT("[Payload] MessageId=%d Title=\"%s\" Text=\"%s\" CreatedAt=\"%s\""),
				       Payload.MessageId,
				       *Payload.Title, *Payload.Text, *Payload.CreatedAt.ToString()
				);
				UE_LOG(LogTemp, Log,
				       TEXT("[Payload] NeedConfirm=%s IsConfirmed=%s"),
				       Payload.NeedConfirmation ? TEXT("true") : TEXT("false"),
				       Payload.IsConfirmed ? TEXT("true") : TEXT("false")
				);
			}

			for (int32 Index = 0; Index < DispatchHistory.Items.Num(); ++Index)
			{
				const FDispatchItem& Item = DispatchHistory.Items[Index];
				const FDispatchPayloadBase& Payload = Item.Payload;
				if (Item.Event == TEXT("notice"))
				{
					NoticeUI->AddNotice(Payload);
					if (Payload.IsConfirmed == false)
					{
						owner->PlayerUI->NewNoticeBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
					}
				}
				else if (Item.Event == TEXT("dm"))
				{
					NoticeUI->AddDM(Payload);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Unknown Event : %s"), *Item.Event);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("DispatchHistory 파싱 실패"));
		}
		NoticeUI->SortNotices(NoticeUI->ConfirmedVBox, NoticeUI->UnConfirmedVBox);
		NoticeUI->SortDMs();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DispatchHistory Response 실패 : %s"), *Message);
	}
}

void UPlayerNoticeComponent::OnNotice(const FDispatchPayloadBase& Notice)
{
	if (NoticeUI)
	{
		owner->AudioManager->PlayNoticeSound();
		EnqueueDispatch(Notice);
		NoticeUI->AddNotice(Notice);

		if (Notice.IsConfirmed == false)
		{
			owner->PlayerUI->NewNoticeBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}

void UPlayerNoticeComponent::OnDirectMessage(const FDispatchPayloadBase& DM)
{
	Server_OnSendDM(DM);
}

void UPlayerNoticeComponent::EnqueueDispatch(const FDispatchPayloadBase& Dispatch)
{
	DispatchQueue.Enqueue(Dispatch);

	if (bIsDisplaying == false)
	{
		DisplayNextDispatch();
	}
}

void UPlayerNoticeComponent::DisplayNextDispatch()
{
	if (DispatchQueue.IsEmpty())
	{
		bIsDisplaying = false;
		NoticeUI->HideDispatchPopUp();
		return;
	}

	bIsDisplaying = true;
	FDispatchPayloadBase Dispatch;
	DispatchQueue.Dequeue(Dispatch);
	NoticeUI->DisplayDispatchPopUp(Dispatch);

	GetWorld()->GetTimerManager().SetTimer(
		DispatchDisplayTimer,
		this,
		&UPlayerNoticeComponent::DisplayNextDispatch,
		DisplayTime,
		false
	);
}

void UPlayerNoticeComponent::Server_OnSendDM_Implementation(const FDispatchPayloadBase& DM)
{
	for (APlayerState* PS : owner->GS->PlayerArray)
	{
		AMumulPlayerState* MPS = Cast<AMumulPlayerState>(PS);
		if (MPS->PS_UserIndex == DM.RecipientId)
		{
			Client_OnSendDM(DM);
		}
	}
}

void UPlayerNoticeComponent::Client_OnSendDM_Implementation(const FDispatchPayloadBase& DM)
{
	if (NoticeUI)
	{
		owner->AudioManager->PlayNoticeSound();
		EnqueueDispatch(DM);
		NoticeUI->AddDM(DM);
	}
}
