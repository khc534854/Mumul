// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MumulGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerArrayUpdated);

USTRUCT(BlueprintType)
struct FChatBlock
{
	GENERATED_BODY()

private:
	FString TimeStamp;

	FString PlayerName;

	FString Content;

public:
	void SetContent(FString Time, FString Player, FString Text)
	{
		TimeStamp = Time;
		PlayerName = Player;
		Content = Text;
	}

	void GetContent(FString& Time, FString& Player, FString& Text)
	{
		Time = TimeStamp;
		Player = PlayerName;
		Text = Content;
	}
};

UCLASS()
class MUMUL_API AMumulGameState : public AGameState
{
	GENERATED_BODY()

protected:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

public:
	UPROPERTY()
	FPlayerArrayUpdated OnPlayerArrayUpdated;

public:
	// [핵심] 모든 클라이언트에게 "네 컴퓨터에 텐트 정보 저장해!"라고 명령하는 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SaveTentData(int32 UserIndex, FTransform TentTransform);

protected:
	TMap<FString, TArray<FChatBlock>> GroupChatHistory;

public:
	TMap<FString, TArray<FChatBlock>> GetGroupChatHistory() { return GroupChatHistory; }
	UFUNCTION(Server, Reliable)
	void Server_RequestGroupChatHistory(const FString& GroupName);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_AddGroupChatHistory(const FString& GroupName);
	UFUNCTION(Server, Reliable)
	void Server_RequestChatHistory(const FString& GroupName, const FString& Time, const FString& Player, const FString& Text);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_InsertChatHistory(const FString& GroupName, const FString& Time, const FString& Player, const FString& Text);
};
