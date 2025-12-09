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
	virtual void Logout(AController* Exiting) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY()
	TSubclassOf<class ATentActor> TentClass;
	UPROPERTY(EditDefaultsOnly)
	int32 PoolSize = 6;
	UPROPERTY()
	TMap<TObjectPtr<class ATentActor>, int32> TentPool;

	// 저장 로직이 중복되므로 함수로 분리
	void SaveUserData(AController* Controller);
public:
	void SpawnTent(const FTransform& SpawnTransform, int32 UserIndex, bool bSaveToDisk);
};
