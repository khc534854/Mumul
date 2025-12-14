// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AnswerCommentaryUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UAnswerCommentaryUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBorder> CommentaryBorder;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> QuestionText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> AnswerText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> CommentaryText;
	
public:
	void SetCommentaryColor(bool TrueGreenOrFalseRed);
	void SetQuestion(const FString& NewQuestion);
	void SetAnswer(const bool& TrueCorrectOrFalseWrong);
	void SetCommentary(const FString& NewCommentary);
};
