// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OXQuizUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UOXQuizUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UWidgetSwitcher> OXQuizWS;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> QuizSizeBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> TimerIMG;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> QuizTimerText;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UQuizQuestionUI> QuizQuestionUIClass;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UQuizAnswerUI> QuizAnswerUIClass;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> QuizResultSizeBox;
	
public:
	void SwitchQuizState(const bool& QuizOrResult);
	
	void SetQuizQuestion(const FString& NewQuiz);
	void SetQuizAnswer(const bool& AnswerResult, const bool& NewAnswer, const FString& NewCommentary);
	
	void SetTimerText(const FString& NewTime);
};
