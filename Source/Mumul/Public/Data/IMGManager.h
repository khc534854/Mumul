// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "IMGManager.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UIMGManager : public UObject
{
	GENERATED_BODY()
	UIMGManager();
	
protected:
	UPROPERTY()
	class UTeamIconDataAsset* TeamIMGAsset;
	UPROPERTY()
	class UUserIconDataAsset* UserIMGAsset;
	
public:
	UTexture2D* GetImageByTeamID(const FString& TeamID);
	UTexture2D* GetImageByUserID(const int32& UserID);
};
