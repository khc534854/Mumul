// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OXQuizPlayerFinderActor.generated.h"

UCLASS()
class MUMUL_API AOXQuizPlayerFinderActor : public AActor
{
	GENERATED_BODY()
	AOXQuizPlayerFinderActor();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY()
	TObjectPtr<class AMumulMumulGameMode> GM;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USphereComponent> PlayerFinderSphere;

public:
	void CheckParticipatingPlayers();
};
