// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/MumulGameState.h"
#include "GameFramework/PlayerState.h"
#include "khc/Save/MapDataSaveGame.h"
#include "Kismet/GameplayStatics.h"

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

void AMumulGameState::Multicast_SaveTentData_Implementation(int32 UserIndex, FTransform TentTransform)
{
	FString SlotName = TEXT("IslandMapSave"); // 세이브 파일 이름 고정

	// 1. 기존 세이브 파일 로드 (없으면 생성)
	UMapDataSaveGame* SaveInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveInst)
	{
		SaveInst = Cast<UMapDataSaveGame>(UGameplayStatics::CreateSaveGameObject(UMapDataSaveGame::StaticClass()));
	}

	// 2. 데이터 갱신 (이미 있는 유저라면 위치만 수정, 없으면 추가)
	bool bFound = false;
	for (FTentSaveData& Data : SaveInst->SavedTents)
	{
		if (Data.OwnerUserIndex == UserIndex)
		{
			Data.Transform = TentTransform; // 위치 업데이트
			bFound = true;
			break;
		}
	}

	if (!bFound)
	{
		FTentSaveData NewData;
		NewData.OwnerUserIndex = UserIndex;
		NewData.Transform = TentTransform;
		SaveInst->SavedTents.Add(NewData);
	}

	// 3. 파일에 쓰기 (저장)
	if (UGameplayStatics::SaveGameToSlot(SaveInst, SlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("[SaveGame] Tent Saved for User %d"), UserIndex);
	}
}
