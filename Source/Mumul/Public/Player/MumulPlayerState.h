// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MumulPlayerState.generated.h"


USTRUCT(Blueprintable)
struct FTeamData
{
	GENERATED_BODY()

	UPROPERTY()
	FString UniqueTeamID;

	UPROPERTY()
	FString TeamName;

	//UPROPERTY()
	//int32 TeamLeaderID;

	UPROPERTY()
	TArray<int32> TeamMateList;
};

UCLASS()
class MUMUL_API AMumulPlayerState : public APlayerState
{
	GENERATED_BODY()

	// Player Information
public:
	UPROPERTY(ReplicatedUsing = OnRep_UserIndex, BlueprintReadOnly, Category = "User Info")
	int32 PS_UserIndex = 0; // userId
	UFUNCTION()
	void OnRep_UserIndex();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "User Info")
	FString PS_RealName; // name

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "User Info")
	FString PS_UserType; // userType

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "User Info")
	int32 PS_TendencyID = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "User Info")
	TArray<FTeamData> PS_PlayerTeamList;

	// 현재 보이스 채널 ID (Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_VoiceChannelID, BlueprintReadOnly, Category = "Voice")
	FString VoiceChannelID = TEXT("None"); // 0: 로비, 1~N: 특정 채널

	FString WaitingChannelID = TEXT("Lobby");

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	bool bIsTentInstalled;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	bool bIsNearByCampFire = false;

	// 채널 변경 요청 (서버에서 실행)
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SetVoiceChannelID(const FString& NewChannelID);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ID가 변경되면 호출되는 함수 (클라이언트)
	UFUNCTION()
	void OnRep_VoiceChannelID();
};
