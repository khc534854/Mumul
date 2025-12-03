// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MumulGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerArrayUpdated);


UCLASS()
class MUMUL_API AMumulGameState : public AGameState
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

public:
	UPROPERTY()
	FPlayerArrayUpdated OnPlayerArrayUpdated;

public:
	// [핵심] 모든 클라이언트에게 "네 컴퓨터에 텐트 정보 저장해!"라고 명령하는 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SaveTentData(int32 UserIndex, FTransform TentTransform);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SavePlayerLocation(int32 UserIndex, FTransform Location);

protected:
	UPROPERTY(Replicated, BlueprintReadWrite)
	TArray<FString> TeamChatList;
public:
	UFUNCTION(Server, Reliable)
	void Server_AddTeamChatList(const FString& TeamName);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddTeamChatList(const FString& TeamName);
	TArray<FString> GetTeamChatList() { return TeamChatList; }

public:
	// 진행 중인 회의 목록 (Key: 채널ID, Value: 미팅ID)
	TMap<int32, FString> ActiveMeetings;

	// 회의 등록/해제 함수
	void RegisterMeeting(int32 ChannelID, FString MeetingID);
	void UnregisterMeeting(int32 ChannelID);
	FString GetActiveMeetingID(int32 ChannelID);
};
