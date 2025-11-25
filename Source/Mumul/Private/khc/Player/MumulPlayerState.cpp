// Fill out your copyright notice in the Description page of Project Settings.


#include "khc/Player/MumulPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Yeomin/Player/CuteAlienController.h"

void AMumulPlayerState::Server_SetVoiceChannelID_Implementation(int32 NewChannelID)
{
	VoiceChannelID = NewChannelID;

	if (GetNetMode() != NM_Client) 
	{
		OnRep_VoiceChannelID();
	}
}

void AMumulPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 변수 동기화 등록
	DOREPLIFETIME(AMumulPlayerState, VoiceChannelID);
}

void AMumulPlayerState::OnRep_VoiceChannelID()
{
	APlayerController* LocalPC = GetGameInstance()->GetFirstLocalPlayerController();
	if (ACuteAlienController* MyPC = Cast<ACuteAlienController>(LocalPC))
	{
		MyPC->UpdateVoiceChannelMuting();
	}
}
