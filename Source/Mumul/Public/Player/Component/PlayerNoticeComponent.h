// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerNoticeComponent.generated.h"


struct FDispatchPayloadBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUMUL_API UPlayerNoticeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerNoticeComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY()
	TObjectPtr<class UHttpNetworkSubsystem> HttpSystem;
	UFUNCTION()
	void OnServerDispatchHistoryResponse(bool bSuccess, FString Message);
	
	UPROPERTY()
	TObjectPtr<class UWebSocketSubsystem> WebSocketSystem;
	UFUNCTION()
	void OnNotice(const FDispatchPayloadBase& Notice);
	UFUNCTION()
	void OnDirectMessage(const FDispatchPayloadBase& DM);
	UFUNCTION(Server, Reliable)
	void Server_OnSendDM(const FDispatchPayloadBase& DM);
	UFUNCTION(Client, Reliable)
	void Client_OnSendDM(const FDispatchPayloadBase& DM);
	
	TQueue<FDispatchPayloadBase> DispatchQueue;
	bool bIsDisplaying = false;
	UPROPERTY(EditDefaultsOnly)
	float DisplayTime = 5.f;
	FTimerHandle DispatchDisplayTimer;
	void EnqueueDispatch(const FDispatchPayloadBase& Dispatch);
	void DisplayNextDispatch();
	
public:
	UPROPERTY()
	TObjectPtr<class UNoticeUI> NoticeUI;
	
protected:
	UPROPERTY()
	TObjectPtr<class ACuteAlienController> owner;
	UPROPERTY()
	TObjectPtr<class ACuteAlienPlayer> player;
};
