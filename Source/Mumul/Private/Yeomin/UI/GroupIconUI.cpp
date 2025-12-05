// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/GroupIconUI.h"

#include "HttpNetworkSubsystem.h"
#include "Yeomin/UI/ChatBlockUI.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "khc/Player/MumulPlayerState.h"
#include "khc/System/NetworkStructs.h"
#include "Yeomin/UI/ChatMessageBlockUI.h"
#include "Yeomin/UI/GroupChatUI.h"

void UGroupIconUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>();
	if (HttpSystem)
	{
		HttpSystem->OnTeamChatMessageResponse.AddDynamic(this, &UGroupIconUI::OnServerTeamChatMessageResponse);
	}
	
	ChatBlockUI = CreateWidget<UChatBlockUI>(GetWorld(), ChatBlockUIClass);
	GroupIconBtn->OnPressed.AddDynamic(this, &UGroupIconUI::DisplayGroupChat);
}

void UGroupIconUI::DisplayGroupChat()
{
	ParentUI->RemoveChatBlock();
	ParentUI->AddChatBlock(ChatBlockUI);

	if (AMumulPlayerState* PS = GetOwningPlayer()->GetPlayerState<AMumulPlayerState>())
	{
		if (PS->bIsNearByCampFire)
		{
			PS->Server_SetVoiceChannelID(ChatBlockUI->GetTeamID());
		}
		else
		{
			PS->WaitingChannelID = ChatBlockUI->GetTeamID();
		}
	}
	
	
	ParentUI->SetGroupNameTitle(ChatBlockUI->GetTeamName());
	
	// Send TeamChatMessage Request
	HttpSystem->SendTeamChatMessageRequest(ChatBlockUI->GetTeamID());
}

void UGroupIconUI::InitParentUI(UGroupChatUI* Parent)
{
	ParentUI = Parent;
}

void UGroupIconUI::SetIconIMG(UTexture2D* IMG)
{
	FSlateBrush Brush;
	Brush.SetResourceObject(IMG);
	Brush.ImageSize = FVector2D(120.f);

	FButtonStyle Style;
	Style.Normal = Brush;
	Style.Hovered = Brush;
	Style.Pressed = Brush;
	
	GroupIconBtn->SetStyle(Style);
}

void UGroupIconUI::OnServerTeamChatMessageResponse(bool bSuccess, FString Message)
{
	if (bSuccess)
	{
		// Init ChatBlockUI
		ChatBlockUI->ChatScrollBox->ClearChildren();
		
		// 1. JSON 파싱 (Message에는 JSON 원본이 들어있음)
		TArray<FTeamChatMessageResponse> TeamChatMessage;

		if (FJsonObjectConverter::JsonArrayStringToUStruct(Message, &TeamChatMessage, 0, 0))
		{
			// JSON Parsing LOG
			for (const FTeamChatMessageResponse& Msg : TeamChatMessage)
			{
				UE_LOG(LogTemp, Warning, TEXT("======== Chat Message ========"));
				UE_LOG(LogTemp, Warning, TEXT("chatId     : %s"), *Msg.chatId);
				UE_LOG(LogTemp, Warning, TEXT("userId     : %d"), Msg.userId);
				UE_LOG(LogTemp, Warning, TEXT("userName   : %s"), *Msg.userName);
				UE_LOG(LogTemp, Warning, TEXT("message    : %s"), *Msg.message);
				UE_LOG(LogTemp, Warning, TEXT("createdAt  : %s"), *Msg.createdAt);
			}
			
			for (const FTeamChatMessageResponse& Msg : TeamChatMessage)
			{
				// Add Chat Chunk to ScrollBox
				UChatMessageBlockUI* Chat = CreateWidget<UChatMessageBlockUI>(GetWorld(), ChatMessageBlockUIClass);
				ChatBlockUI->ChatScrollBox->AddChild(Chat);
				Chat->SetContent(*Msg.createdAt, *Msg.userName, *Msg.message);
				Chat->SetChatID(Msg.chatId);
				Chat->SetUserID(Msg.userId);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("TeamChatMessage 파싱 실패"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TeamChatMessage Response 실패 : %s"), *Message);
	}
}
