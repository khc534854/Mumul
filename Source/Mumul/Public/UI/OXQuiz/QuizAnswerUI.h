// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuizAnswerUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UQuizAnswerUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBorder> AnswerBorder;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> JudgingAnswerText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> AnswerText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> CommentaryText;
	
public:
	void SetAnswerColor(bool TrueGreenOrFalseRed);
	void SetAnswerResult(bool TrueCorrectOrFalseWrong);
	void SetQuizAnswer(bool TrueCorrectOrFalseWrong);
	void SetAnswerCommentary(const FString& NewCommentary);
};
