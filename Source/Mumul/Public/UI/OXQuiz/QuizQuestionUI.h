// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuizQuestionUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UQuizQuestionUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> QuestionText;
	
public:
	void SetQuestionText(const FString& Text);
};
