// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Data/IMGManager.h"

#include "Yeomin/Data/TeamIconDataAsset.h"

UIMGManager::UIMGManager()
{
	static ConstructorHelpers::FObjectFinder<UTeamIconDataAsset> DataAssetObj(
		TEXT("/Game/Yeomin/Data/DA_TeamIcon.DA_TeamIcon"));
	if (DataAssetObj.Succeeded())
	{
		ImageAsset = DataAssetObj.Object;
	}
}

UTexture2D* UIMGManager::GetNextImage()
{
		if (!ImageAsset || ImageAsset->TeamIconList.Num() == 0)
			return nullptr;

		UTexture2D* IMG = ImageAsset->TeamIconList[CurrentIndex];
		CurrentIndex = (CurrentIndex + 1) % ImageAsset->TeamIconList.Num();
		return IMG;
}
