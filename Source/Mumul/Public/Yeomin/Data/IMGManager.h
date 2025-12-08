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
	

public:
	UPROPERTY()
	class UTeamIconDataAsset* ImageAsset;

	int32 CurrentIndex = 0;
	
	UTexture2D* GetImageByTeamID(const FString& TeamID);
};
