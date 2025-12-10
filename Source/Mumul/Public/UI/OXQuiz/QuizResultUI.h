// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuizResultUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UQuizResultUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> AnswerListScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ConfirmBtn;
};
