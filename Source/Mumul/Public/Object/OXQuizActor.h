// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OXQuizActor.generated.h"

UCLASS()
class MUMUL_API AOXQuizActor : public AActor
{
	GENERATED_BODY()
	AOXQuizActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY()
	TObjectPtr<class AMumulMumulGameMode> GM;
	
	UPROPERTY()
	TObjectPtr<class UArrowComponent> LineArrow;
	UPROPERTY()
	TObjectPtr<class UBoxComponent> ParticipateBox;
	
	UFUNCTION(Server, Reliable)
	void Server_StartOXQuiz(const FString& Question, const FString& Difficulty);
	void CheckParticipatingPlayers();
public:
	void JudgePlayerAnswers();
};
