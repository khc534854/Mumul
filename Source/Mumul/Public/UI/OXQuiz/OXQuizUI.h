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
	TObjectPtr<class USizeBox> QuizSizeBox;
};
