// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MumulPlayerState.h"

#include "Base/MumulGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/CuteAlienController.h"

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
				UE_LOG(LogTemp, Warning, TEXT("[Server] User %s joined active meeting channel %s. Auto-joining..."), *GetPlayerName(), *NewChannelID);
                
				// 컨트롤러에게 "너도 빨리 참가해!" 명령 (기존 함수 재활용)
				PC->Client_RequestJoinMeeting(ActiveMeetingID);
			}
		}
	}
}

void AMumulPlayerState::OnRep_UserIndex()
{
	// TeamList, UserProfileList 초기화
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		ACuteAlienController* CAPC = Cast<ACuteAlienController>(PC);
		if (CAPC)
		{
			CAPC->Server_InitPlayerArray();
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
}

void AMumulPlayerState::OnRep_VoiceChannelID()
{
	APlayerController* LocalPC = GetGameInstance()->GetFirstLocalPlayerController();
	if (ACuteAlienController* MyPC = Cast<ACuteAlienController>(LocalPC))
	{
		MyPC->UpdateVoiceChannelMuting();
	}
}
