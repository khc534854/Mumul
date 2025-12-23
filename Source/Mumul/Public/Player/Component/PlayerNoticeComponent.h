// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerNoticeComponent.generated.h"


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
	TObjectPtr<class UWebSocketSubsystem> WebSocketSystem;
	UFUNCTION()
	void OnNotice(const FDispatchNoticePayload& Notice);
	UFUNCTION()
	void OnDirectMessage(const FDispatchDMPayload& DM);
	
public:
	UPROPERTY()
	TObjectPtr<class UNoticeUI> NoticeUI;
	
protected:
	UPROPERTY()
	TObjectPtr<class ACuteAlienController> owner;
	UPROPERTY()
	TObjectPtr<class ACuteAlienPlayer> player;
};
