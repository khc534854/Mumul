// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/MumulGameState.h"

#include "EngineUtils.h"
#include "Components/ScrollBox.h"
#include "GameFramework/PlayerState.h"
#include "Save/MapDataSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Object/Tent/TentActor.h"
#include "Player/CuteAlienController.h"
#include "Player/Component/PlayerChatComponent.h"
#include "UI/ChatBlockUI.h"
#include "UI/GroupChatUI.h"
#include "UI/GroupIconUI.h"


void AMumulGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AMumulGameState, TeamChatList)
	DOREPLIFETIME(AMumulGameState, ActiveMeetings)
}

void AMumulGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

}

void AMumulGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);

}

void AMumulGameState::Multicast_SaveTentData_Implementation(int32 UserIndex, const FString& OwnerName,
	FTransform TentTransform)
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
			Data.OwnerName = OwnerName;
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
		NewData.OwnerName = OwnerName;
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

void AMumulGameState::Multicast_SavePlayerCosmetic_Implementation(int32 UserIndex, FName ItemID)
{
	FString SlotName = TEXT("IslandMapSave");

	// 1. 로드
	UMapDataSaveGame* SaveInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveInst)
	{
		SaveInst = Cast<UMapDataSaveGame>(UGameplayStatics::CreateSaveGameObject(UMapDataSaveGame::StaticClass()));
	}

	// 2. 데이터 갱신
	if (ItemID == NAME_None)
	{
		// 해제 시 맵에서 제거하거나 None으로 저장
		SaveInst->PlayerCosmetics.Remove(UserIndex);
	}
	else
	{
		// 맵에 추가/덮어쓰기
		SaveInst->PlayerCosmetics.Add(UserIndex, ItemID);
	}

	// 3. 저장
	if (UGameplayStatics::SaveGameToSlot(SaveInst, SlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("[SaveGame] User %d Equipped Item: %s"), UserIndex, *ItemID.ToString());
	}
}

void AMumulGameState::Multicast_SavePlayerTendency_Implementation(int32 UserIndex, int32 TendencyID)
{
	FString SlotName = TEXT("IslandMapSave");

	UMapDataSaveGame* SaveInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveInst)
	{
		SaveInst = Cast<UMapDataSaveGame>(UGameplayStatics::CreateSaveGameObject(UMapDataSaveGame::StaticClass()));
	}

	SaveInst->PlayerTendency.Add(UserIndex, TendencyID);

	if (UGameplayStatics::SaveGameToSlot(SaveInst, SlotName, 0))
	{
		UE_LOG(LogTemp, Log, TEXT("[SaveGame] User %d Tendency : %d"), UserIndex, TendencyID);
	}
}

void AMumulGameState::OnRep_ActiveMeetings()
{
	ACuteAlienController* PC = Cast<ACuteAlienController>(GetWorld()->GetFirstPlayerController());
	if (!PC || !PC->ChatComp || !PC->ChatComp->GroupChatUI) return;

	UGroupChatUI* ChatUI = PC->ChatComp->GroupChatUI;

	// 2. UI에 있는 모든 그룹 아이콘을 순회하며 상태 동기화
	// (ActiveMeetings에 있으면 ON, 없으면 OFF)
	if (ChatUI->GroupScrollBox)
	{
		for (UWidget* Child : ChatUI->GroupScrollBox->GetAllChildren())
		{
			if (UGroupIconUI* IconUI = Cast<UGroupIconUI>(Child))
			{
				if (!IconUI->ChatBlockUI) continue;
                
				FString TeamID = IconUI->ChatBlockUI->GetTeamID();
                
				// ActiveMeetings 리스트에 이 팀 ID가 있는지 확인
				bool bIsActive = false;
				for (const FMeetingInfo& Info : ActiveMeetings)
				{
					if (Info.ChannelID == TeamID)
					{
						bIsActive = true;
						break;
					}
				}

				// 3. 아이콘 상태 갱신 (ON / OFF)
				IconUI->SetMeetingStatus(bIsActive);

				// 4. 내가 현재 보고 있는 방이라면 버튼 등 상세 UI도 갱신
				if (ChatUI->GetCurrentTeamID() == TeamID)
				{
					ChatUI->UpdateQuestionButtonState();
					ChatUI->OnRecordBtnState(bIsActive);
				}
			}
		}
	}
    
	// 5. 로컬 캐시(ActiveMeetingTeams)도 갱신 (ChatComp에 있다면)
	PC->ChatComp->ActiveMeetingTeams.Empty();
	for (const FMeetingInfo& Info : ActiveMeetings)
	{
		PC->ChatComp->ActiveMeetingTeams.Add(Info.ChannelID);
	}
}

void AMumulGameState::RegisterMeeting(FString ChannelID, FString MeetingID)
{
	if (GetLocalRole() != ROLE_Authority) return;

	// 1. 이미 등록된 채널인지 확인 (있으면 ID 업데이트)
	bool bFound = false;
	for (FMeetingInfo& Info : ActiveMeetings)
	{
		if (Info.ChannelID == ChannelID)
		{
			Info.MeetingID = MeetingID;
			bFound = true;
			break;
		}
	}

	// 2. 없으면 새로 추가
	if (!bFound)
	{
		FMeetingInfo NewInfo;
		NewInfo.ChannelID = ChannelID;
		NewInfo.MeetingID = MeetingID;
		ActiveMeetings.Add(NewInfo);
	}

	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_ActiveMeetings();
	}

	// 배열 내부 값이 바뀌었으므로 리플리케이션 자동 발동 (서버 -> 클라)
	UE_LOG(LogTemp, Warning, TEXT("[GameState] Meeting Registered: Ch %s -> ID %s"), *ChannelID, *MeetingID);
}

void AMumulGameState::UnregisterMeeting(FString ChannelID)
{
	if (GetLocalRole() != ROLE_Authority) return;

	// 람다식을 이용해 조건에 맞는 항목 제거
	int32 RemovedCount = ActiveMeetings.RemoveAll([&](const FMeetingInfo& Info) {
		return Info.ChannelID == ChannelID;
	});

	if (RemovedCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameState] Meeting Ended for Ch %s"), *ChannelID);
	}

	if (GetNetMode() == NM_ListenServer)
	{
		OnRep_ActiveMeetings();
	}
}

FString AMumulGameState::GetActiveMeetingID(FString ChannelID)
{
	for (const FMeetingInfo& Info : ActiveMeetings)
	{
		if (Info.ChannelID == ChannelID)
		{
			return Info.MeetingID;
		}
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

void AMumulGameState::AddTeamChatList(const FString& TeamID)
{
	TeamChatList.Add(TeamID);
}
