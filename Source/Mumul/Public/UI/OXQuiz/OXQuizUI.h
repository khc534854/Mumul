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
	
	void SetTimerText(const FString& NewTime);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> AnswerListScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ConfirmBtn;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UAnswerCommentaryUI> AnswerCommentaryUIClass;
	
public:
	void SwitchQuizState(const bool& QuizOrResult);
	
	void SetQuizQuestion(const FString& NewQuiz);
	
	void SetQuizAnswer(const bool& AnswerResult, const bool& NewAnswer, const FString& NewCommentary);
	
	void StartQuestionTimer();
	void StartAnswerTimer();
	
	void SetQuizResult(const bool& AnswerResult, const FString& QuestionText, const bool& AnswerText, const FString& CommentaryText);
};
