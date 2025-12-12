// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Network/NetworkStructs.h"
#include "MumulMumulGameMode.generated.h"

/**
 * 
 */
UENUM()
enum class EQuizPhase : uint8
{
	Question,
	Answer,
};

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
	TObjectPtr<class AMumulGameState> GS;
		
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
	
	TMap<TObjectPtr<class ACuteAlienController>, TArray<bool>> ParticipatingPlayers;
	void RegisterQuizActor(class AOXQuizActor* InActor);
protected:
	UPROPERTY()
	TObjectPtr<class AOXQuizActor> OXQuizActor;
	FLearningQuizResponse LearningQuiz;
	
	UFUNCTION()
	void OnServerLearningQuizResponse(bool bSuccess, FString Message);
	
	EQuizPhase QuizPhase = EQuizPhase::Question;
	
	int32 CurrentQuizIdx;
	int32 MaxQuizCount;
	UPROPERTY(EditAnywhere, Category="OXQuiz Time")
	int32 QuestionTime = 5;
	UPROPERTY(EditAnywhere, Category="OXQuiz Time")
	int32 AnswerTime = 2;
	FTimerHandle QuizTimer;
	void StartLearningQuiz();
	void StartQuestionPhase();
	void EnterNextStep();
	void StartAnswerPhase();
	
	void ShowResult();
};
