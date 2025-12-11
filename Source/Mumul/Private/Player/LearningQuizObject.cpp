// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LearningQuizObject.h"

#include "Network/HttpNetworkSubsystem.h"
#include "Network/NetworkStructs.h"


void ULearningQuizObject::Initialize(UGameInstance* InGameInstance)
{
	UHttpNetworkSubsystem* HttpSystem = InGameInstance->GetSubsystem<UHttpNetworkSubsystem>();
	if (HttpSystem)
	{
		HttpSystem->OnLearningQuizResponse.AddDynamic(this, &ULearningQuizObject::OnServerLearningQuizResponse);
	}
}

void ULearningQuizObject::OnServerLearningQuizResponse(bool bSuccess, FString Message)
{
	if (bSuccess)
	{
		// 1. JSON 파싱 (Message에는 JSON 원본이 들어있음)
		FLearningQuizResponse LearningQuiz;

		if (FJsonObjectConverter::JsonObjectStringToUStruct(Message, &LearningQuiz, 0, 0))
		{
			// JSON Parsing LOG
			UE_LOG(LogTemp, Log, TEXT("isLearningQuestion: %s"), LearningQuiz.isLearningQuestion ? TEXT("true") : TEXT("false"));
			UE_LOG(LogTemp, Log, TEXT("grade: %s"), *LearningQuiz.grade);

			UE_LOG(LogTemp, Log, TEXT("Quiz Count: %d"), LearningQuiz.quiz.Num());
			for (int32 i = 0; i < LearningQuiz.quiz.Num(); i++)
			{
				const FQuizContent& Content = LearningQuiz.quiz[i];
				UE_LOG(LogTemp, Log, TEXT("---- Quiz %d ----"), i + 1);
				UE_LOG(LogTemp, Log, TEXT("id: %d"), Content.id);
				UE_LOG(LogTemp, Log, TEXT("type: %s"), *Content.type);
				UE_LOG(LogTemp, Log, TEXT("question: %s"), *Content.question);
				UE_LOG(LogTemp, Log, TEXT("answer: %s"), *Content.answer);
				UE_LOG(LogTemp, Log, TEXT("explanation: %s"), *Content.explanation);
			}
			
			StartLearningQuiz(LearningQuiz.quiz.Num());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("LearningQuiz 파싱 실패"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LearningQuiz Response 실패 : %s"), *Message);
	}
}

void ULearningQuizObject::StartLearningQuiz(const int32& QuestionNumber)
{
	
}