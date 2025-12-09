// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TeamIconDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UTeamIconDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UTexture2D>> TeamIconList;
};
