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
	int32 UniqueTeamID;

	UPROPERTY()
	FString TeamName;

	UPROPERTY()
	int32 TeamLeaderID;

	UPROPERTY()
	TSet<int32> TeamMateList;
};

UCLASS()
class MUMUL_API AMumulPlayerState : public APlayerState
{
	GENERATED_BODY()

	// Player Information
public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "User Info")
	int32 PS_UserIndex = 0; // userId

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
	int32 VoiceChannelID = 0; // 0: 로비, 1~N: 특정 채널

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
	bool bIsTentInstalled;

	// 채널 변경 요청 (서버에서 실행)
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SetVoiceChannelID(int32 NewChannelID);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ID가 변경되면 호출되는 함수 (클라이언트)
	UFUNCTION()
	void OnRep_VoiceChannelID();
};
