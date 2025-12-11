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
	TObjectPtr<class UBaseText> QuestionText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> AnswerText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> CommentaryText;
};
