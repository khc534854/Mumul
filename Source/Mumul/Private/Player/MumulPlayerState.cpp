// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MumulPlayerState.h"

#include "Base/MumulGameState.h"
#include "Data/IMGManager.h"
#include "Net/UnrealNetwork.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "Player/Component/PlayerChatComponent.h"
#include "Player/Component/PlayerMeetingManagerComponent.h"
#include "UI/CreateGroupChatUI.h"
#include "UI/GroupChatUI.h"
#include "UI/InvitationUI.h"
#include "UI/PlayerUI.h"


void AMumulPlayerState::BeginPlay()
{
	Super::BeginPlay();

	IMGManager = NewObject<UIMGManager>(this, UIMGManager::StaticClass());

	HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>();
}

void AMumulPlayerState::Server_SetVoiceChannelID_Implementation(const FString& NewChannelID)
{
	VoiceChannelID = NewChannelID;

	if (GetNetMode() != NM_Client)
	{
		OnRep_VoiceChannelID();
	}

	if (AMumulGameState* GS = GetWorld()->GetGameState<AMumulGameState>())
	{
		FString ActiveMeetingID = GS->GetActiveMeetingID(NewChannelID);

		// 회의 중인 방에 들어왔다면?
		if (!ActiveMeetingID.IsEmpty())
		{
			if (ACuteAlienController* PC = Cast<ACuteAlienController>(GetOwner()))
			{
				UE_LOG(LogTemp, Warning, TEXT("[Server] User %s joined active meeting channel %s. Auto-joining..."),
				       *GetPlayerName(), *NewChannelID);

				// 컨트롤러에게 "너도 빨리 참가해!" 명령 (기존 함수 재활용)
				PC->MeetingComp->Client_RequestJoinMeeting(ActiveMeetingID);
			}
		}
	}
}

void AMumulPlayerState::OnRep_UserIndex()
{
	// PlayerProfile, TeamList, Player ProfileList 초기화

	// Get TeamChatList
	ACuteAlienController* PC = Cast<ACuteAlienController>(GetWorld()->GetFirstPlayerController());
	if (PC && PC->PlayerState == this)
	{
		if (HttpSystem)
		{
			HttpSystem->SendTeamChatListRequest(PS_UserIndex);
		}
		// Set PlayerProfile
		// if (PC->IsLocalController())
		// {
		// 	if (PC->PlayerUI)
		// 	{
		// 		if (IMGManager)
		// 		{
		// 			PC->PlayerUI->SetProfileBtnIMG(IMGManager->GetImageByUserID(PS_UserIndex));
		// 		}
		// 	}
		// }
	}

	// Refresh Player ProfileList
	Server_InitPlayerArray();
}

void AMumulPlayerState::Server_InitPlayerArray_Implementation()
{
	Multicast_InitPlayerArray();
}

void AMumulPlayerState::Multicast_InitPlayerArray_Implementation()
{
	if (HttpSystem)
	{
		if (ACuteAlienController* PC = Cast<ACuteAlienController>(GetWorld()->GetFirstPlayerController()))
		{
			if (PC->ChatComp->GroupChatUI && PC->ChatComp->GroupChatUI->CreateGroupChatUI && PC->ChatComp->GroupChatUI->InvitationUI)
			{
				PC->ChatComp->GroupChatUI->CreateGroupChatUI->RefreshJoinedPlayerList();
				PC->ChatComp->GroupChatUI->InvitationUI->RefreshJoinedPlayerList();
			}
		}
	}
}

void AMumulPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 변수 동기화 등록
	DOREPLIFETIME(AMumulPlayerState, VoiceChannelID);
	DOREPLIFETIME(AMumulPlayerState, PS_UserIndex);
	DOREPLIFETIME(AMumulPlayerState, PS_RealName);
	DOREPLIFETIME(AMumulPlayerState, PS_UserType);
	DOREPLIFETIME(AMumulPlayerState, PS_TendencyID);
	DOREPLIFETIME(AMumulPlayerState, PS_PlayerTeamList);
	DOREPLIFETIME(AMumulPlayerState, bIsTentInstalled);
	DOREPLIFETIME(AMumulPlayerState, bIsNearByCampFire);
	DOREPLIFETIME(AMumulPlayerState, EquippedCustomID);
}

void AMumulPlayerState::OnRep_VoiceChannelID()
{
	APlayerController* LocalPC = GetGameInstance()->GetFirstLocalPlayerController();
	if (ACuteAlienController* MyPC = Cast<ACuteAlienController>(LocalPC))
	{
		MyPC->MeetingComp->UpdateVoiceChannelMuting();
	}
}

void AMumulPlayerState::OnRep_EquippedCustomID()
{
	if (ACuteAlienPlayer* Character = Cast<ACuteAlienPlayer>(GetPawn()))
	{
		Character->UpdateCustomMesh(EquippedCustomID);
	}
}

void AMumulPlayerState::OnRep_TendencyID()
{
	if (ACuteAlienPlayer* Character = Cast<ACuteAlienPlayer>(GetPawn()))
	{
		Character->UpdateBodyMaterial(PS_TendencyID);
	}
}

void AMumulPlayerState::OnRep_RealName()
{
	ACuteAlienPlayer* MyPawn = GetPawn<ACuteAlienPlayer>();
    
	if (MyPawn)
	{
		// 캐릭터에게 이름표를 다시 그리라고 명령합니다.
		MyPawn->UpdateNameTag();
	}
}
