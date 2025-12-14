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
	
	UPROPERTY()
	TObjectPtr<class AMumulMumulGameMode> GM;
	
	UPROPERTY()
	TObjectPtr<class AOXQuizPlayerFinderActor> OXQuizPlayerFinder;

public:
	UPROPERTY()
	TObjectPtr<class UArrowComponent> LineArrow;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> FrontBox;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> BackBox;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> LeftBox;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> RightBox;

	void StartOXQuiz(const int32 UserID, const FString& Difficulty);
	void JudgePlayerAnswers();
};
