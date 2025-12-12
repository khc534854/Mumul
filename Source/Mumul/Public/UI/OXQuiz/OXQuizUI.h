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
	virtual void NativeConstruct() override;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UWidgetSwitcher> OXQuizWS;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> QuizSizeBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> TimerIMG;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> QuizTimerText;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UQuizQuestionUI> QuizQuestionUIClass;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UQuizAnswerUI> QuizAnswerUIClass;
	
	FTimerHandle QuizRemainingTimeHandler;
	int32 RemainingTime;
	void SetTimerText(const int32& NewTime);
	void UpdateTimer();
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> AnswerListScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ConfirmBtn;
	UFUNCTION()
	void OnConfirmResult();
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UAnswerCommentaryUI> AnswerCommentaryUIClass;
	
public:
	void SwitchQuizState(const bool& QuizOrResult);
	
	void SetQuizQuestion(const FString& NewQuiz);
	
	void SetQuizAnswer(const bool& AnswerResult, const bool& NewAnswer, const FString& NewCommentary);
	
	void StartQuestionTimer(const int32& QuestionTime);
	void StartAnswerTimer(const int32& QuestionTime);
	
	void SetQuizResult(const bool& AnswerResult, const FString& QuestionText, const bool& AnswerText, const FString& CommentaryText);
};
