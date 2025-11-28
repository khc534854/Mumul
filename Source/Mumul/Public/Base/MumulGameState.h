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
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	
public:
	UPROPERTY()
	FPlayerArrayUpdated OnPlayerArrayUpdated;
};
