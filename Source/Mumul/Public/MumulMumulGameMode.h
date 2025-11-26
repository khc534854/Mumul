// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MumulMumulGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API AMumulMumulGameMode : public AGameMode
{
	GENERATED_BODY()
	AMumulMumulGameMode();

protected:
	virtual void BeginPlay() override;
	UPROPERTY()
	TSubclassOf<class ATentActor> TentClass;
	UPROPERTY(EditDefaultsOnly)
	int32 PoolSize = 6;
	UPROPERTY()
	TMap<TObjectPtr<class ATentActor>, FString> TentPool;

public:
	void SpawnTent(const FTransform& SpawnTransform, FString Name);
};
