// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerOXQuizComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUMUL_API UPlayerOXQuizComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerOXQuizComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UPROPERTY()
	TObjectPtr<class UOXQuizUI> OXQuizUI;
	
	UFUNCTION(Client, Reliable)
	void Client_DisplayReady();
	UFUNCTION(Client, Reliable)
	void Client_HideOXQuizUI();
	int32 CountdownEndTime;
	FTimerHandle ReadyCountdownTimer;
	int32 LastCountdownSecond = TNumericLimits<int32>::Min();
	void UpdateCountdown();
	UFUNCTION(Client, Reliable)
	void Client_StartReadyCountdown(int32 Time);
	UFUNCTION(Client, Reliable)
	void Client_DisplayQuestion(const int32& QuestionIdx, const FString& NewQuestion, const int32& QuestionTime);
	UFUNCTION(Client, Reliable)
	void Client_DisplayAnswer(bool AnswerResult, bool NewAnswer, const FString& NewCommentary, const int32& AnswerTime);
	UFUNCTION(Client, Reliable)
	void Client_DisplayResult(const int32& QuestionIdx, bool AnswerResult, const FString& QuestionText, bool AnswerText, const FString& CommentaryText);
	
	UFUNCTION(Server, Reliable)
	void Server_SpawnCloud(class ACuteAlienPlayer* OwnerPlayer);

	UPROPERTY()
	TObjectPtr<class ACuteAlienController> owner;
	UPROPERTY()
	TObjectPtr<class ACuteAlienPlayer> player;
};
