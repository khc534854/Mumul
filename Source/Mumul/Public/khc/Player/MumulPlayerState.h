// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MumulPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API AMumulPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// 현재 보이스 채널 ID (Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_VoiceChannelID, BlueprintReadOnly, Category = "Voice")
	int32 VoiceChannelID = 0; // 0: 로비, 1~N: 특정 채널

	// 채널 변경 요청 (서버에서 실행)
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SetVoiceChannelID(int32 NewChannelID);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ID가 변경되면 호출되는 함수 (클라이언트)
	UFUNCTION()
	void OnRep_VoiceChannelID();

public:
	FText PlayerName;
};
