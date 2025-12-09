// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/IMGManager.h"

#include "Data/TeamIconDataAsset.h"

UIMGManager::UIMGManager()
{
	static ConstructorHelpers::FObjectFinder<UTeamIconDataAsset> DataAssetObj(
		TEXT("/Game/Yeomin/Data/DA_TeamIcon.DA_TeamIcon"));
	if (DataAssetObj.Succeeded())
	{
		ImageAsset = DataAssetObj.Object;
	}
}

UTexture2D* UIMGManager::GetImageByTeamID(const FString& TeamID)
{
	if (!ImageAsset || ImageAsset->TeamIconList.Num() == 0)
		return nullptr;

	// FString → uint32 해시 변환
	uint32 HashValue = GetTypeHash(TeamID);

	// 아이콘 개수 범위 안에서 고정 인덱스 산출
	int32 IconIndex = HashValue % ImageAsset->TeamIconList.Num();

	return ImageAsset->TeamIconList[IconIndex];
}
