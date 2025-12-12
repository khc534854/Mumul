// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Player/MumulPlayerState.h"
#include "MumulGameState.generated.h"




UCLASS()
class MUMUL_API AMumulGameState : public AGameState
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

public:
	// [핵심] 모든 클라이언트에게 "네 컴퓨터에 텐트 정보 저장해!"라고 명령하는 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SaveTentData(int32 UserIndex, FTransform TentTransform);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SavePlayerLocation(int32 UserIndex, FTransform Location);
	void Multicast_SavePlayerCosmetic(int32 UserIndex, FName ItemID);

protected:
	UPROPERTY(Replicated)
	TArray<FString> TeamChatList;
public:
	TArray<FString> GetTeamChatList() { return TeamChatList; }

public:
	// 진행 중인 회의 목록 (Key: 채널ID, Value: 미팅ID)
	TMap<FString, FString> ActiveMeetings;

	// 회의 등록/해제 함수
	void RegisterMeeting(FString ChannelID, FString MeetingID);
	void UnregisterMeeting(FString ChannelID);
	FString GetActiveMeetingID(FString ChannelID);
	void AddTeamChatList(const FString& TeamID);
};
