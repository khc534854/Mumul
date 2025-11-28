// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/MumulGameState.h"
#include "GameFramework/PlayerState.h"

void AMumulGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	
	UE_LOG(LogTemp, Warning, TEXT("Player Joined: %s"), *PlayerState->GetPlayerName());
	OnPlayerArrayUpdated.Broadcast();
}

void AMumulGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	
	UE_LOG(LogTemp, Warning, TEXT("Player Left: %s"), *PlayerState->GetPlayerName());
	OnPlayerArrayUpdated.Broadcast();
}
