// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/IMGManager.h"

#include "Data/TeamIconDataAsset.h"
#include "Data/UserIconDataAsset.h"

UIMGManager::UIMGManager()
{
	static ConstructorHelpers::FObjectFinder<UTeamIconDataAsset> TeamDataAssetObj(
		TEXT("/Game/Yeomin/Data/DA_TeamIcon.DA_TeamIcon"));
	if (TeamDataAssetObj.Succeeded())
	{
		TeamIMGAsset = TeamDataAssetObj.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<UUserIconDataAsset> UserDataAssetObj(
		TEXT("/Game/Yeomin/Data/DA_UserIcon.DA_UserIcon"));
	if (UserDataAssetObj.Succeeded())
	{
		UserIMGAsset = UserDataAssetObj.Object;
	}
}

UTexture2D* UIMGManager::GetImageByTeamID(const FString& TeamID)
{
	if (!TeamIMGAsset || TeamIMGAsset->TeamIconList.Num() == 0)
		return nullptr;

	// FString → uint32 해시 변환
	uint32 HashValue = GetTypeHash(TeamID);

	// 아이콘 개수 범위 안에서 고정 인덱스 산출
	int32 IconIndex = HashValue % TeamIMGAsset->TeamIconList.Num();

	return TeamIMGAsset->TeamIconList[IconIndex];
}

UTexture2D* UIMGManager::GetImageByUserID(const int32& UserID)
{
	if (!UserIMGAsset || UserIMGAsset->UserIconList.Num() == 0)
		return nullptr;

	// int32는 이미 해시 가능한 값
	uint32 HashValue = GetTypeHash(UserID);

	int32 IconIndex = HashValue % UserIMGAsset->UserIconList.Num();

	return UserIMGAsset->UserIconList[IconIndex];
}
