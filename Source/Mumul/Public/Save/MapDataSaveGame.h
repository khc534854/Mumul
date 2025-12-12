// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MapDataSaveGame.generated.h"


USTRUCT(BlueprintType)
struct FHousingSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FName ItemID;

	UPROPERTY()
	FTransform RelativeTransform;
};

USTRUCT(BlueprintType)
struct FTentSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 OwnerUserIndex = 0; // 텐트 주인 식별용 (Key)

	UPROPERTY()
	FTransform Transform;     // 텐트 위치 및 회전

	UPROPERTY()
	TArray<FHousingSaveData> HousingItems;
};

/**
 * 맵 정보를 저장하는 세이브 게임 클래스
 */
UCLASS()
class MUMUL_API UMapDataSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 맵에 존재하는 모든 텐트 리스트
	UPROPERTY()
	TArray<FTentSaveData> SavedTents;

	UPROPERTY()
	TMap<int32, FTransform> PlayerLocations;

	UPROPERTY()
	TMap<int32, FName> PlayerCosmetics;
};
