// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/MumulGameState.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Save/MapDataSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Object/Tent/TentActor.h"


void AMumulGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMumulGameState, TeamChatList)
}

void AMumulGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

}

void AMumulGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

}

void AMumulGameState::RegisterMeeting(FString ChannelID, FString MeetingID)
{
	ActiveMeetings.Add(ChannelID, MeetingID);
	UE_LOG(LogTemp, Warning, TEXT("[GameState] Meeting Registered: Ch %s -> ID %s"), *ChannelID, *MeetingID);
}

void AMumulGameState::UnregisterMeeting(FString ChannelID)
{
	if (ActiveMeetings.Remove(ChannelID) > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameState] Meeting Ended for Ch %s"), *ChannelID);
	}
}

FString AMumulGameState::GetActiveMeetingID(FString ChannelID)
{
	if (ActiveMeetings.Contains(ChannelID))
	{
		return ActiveMeetings[ChannelID];
	}
	return TEXT("");
}

void AMumulGameState::Multicast_SavePlayerLocation_Implementation(int32 UserIndex, FTransform Location)
{
	FString SlotName = TEXT("IslandMapSave");

	// 1. 로드 (없으면 생성)
	UMapDataSaveGame* SaveInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveInst) SaveInst = Cast<UMapDataSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UMapDataSaveGame::StaticClass()));

	// 2. 맵에 데이터 추가/갱신 (Key가 같으면 덮어씌워짐)
	SaveInst->PlayerLocations.Add(UserIndex, Location);

	// 3. 저장
	if (UGameplayStatics::SaveGameToSlot(SaveInst, SlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("[SaveGame] Location Saved for User %d: %s"), UserIndex,
		       *Location.GetLocation().ToString());
	}
}

void AMumulGameState::Multicast_SaveTentData_Implementation(int32 UserIndex, FTransform TentTransform)
{
	FString SlotName = TEXT("IslandMapSave");

	// 1. 세이브 파일 로드 (없으면 생성)
	UMapDataSaveGame* SaveInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveInst)
	{
		SaveInst = Cast<UMapDataSaveGame>(UGameplayStatics::CreateSaveGameObject(UMapDataSaveGame::StaticClass()));
	}

	// 2. [핵심] 월드에서 해당 유저의 텐트 액터를 찾아 하우징 데이터 가져오기
	TArray<FHousingSaveData> CurrentHousingItems;
	for (TActorIterator<ATentActor> It(GetWorld()); It; ++It)
	{
		ATentActor* Tent = *It;
		// 텐트가 유효하고, 주인이 맞다면
		if (Tent && Tent->OwnerUserIndex == UserIndex)
		{
			// 현재 배치된 하우징 아이템 리스트 복사
			CurrentHousingItems = Tent->HousingItems;
			break; 
		}
	}

	// 3. 세이브 데이터 갱신
	bool bFound = false;
	for (FTentSaveData& Data : SaveInst->SavedTents)
	{
		if (Data.OwnerUserIndex == UserIndex)
		{
			Data.Transform = TentTransform; // 위치 업데이트
			Data.HousingItems = CurrentHousingItems; // [신규] 하우징 아이템 업데이트
			bFound = true;
			break;
		}
	}

	// 데이터가 없으면 새로 추가
	if (!bFound)
	{
		FTentSaveData NewData;
		NewData.OwnerUserIndex = UserIndex;
		NewData.Transform = TentTransform;
		NewData.HousingItems = CurrentHousingItems; // [신규] 하우징 아이템 저장
		SaveInst->SavedTents.Add(NewData);
	}

	// 4. 파일에 쓰기 (저장)
	if (UGameplayStatics::SaveGameToSlot(SaveInst, SlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("[SaveGame] Tent & Housing Items Saved for User %d (Items: %d)"), 
			UserIndex, CurrentHousingItems.Num());
	}
}

void AMumulGameState::AddTeamChatList(const FString& TeamID)
{
	TeamChatList.Add(TeamID);
}
