// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LearningQuizObject.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API ULearningQuizObject : public UObject
{
	GENERATED_BODY()
	
protected:
	void Initialize(UGameInstance* InGameInstance);
	UFUNCTION()
	void OnServerLearningQuizResponse(bool bSuccess, FString Message);
	
	void StartLearningQuiz(const int32& QuestionNumber);
};
