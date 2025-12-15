// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AskOXQuizUI.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnCancelClicked);
/**
 * 
 */
UCLASS()
class MUMUL_API UAskOXQuizUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	FOnCancelClicked OnCancelClicked;
	
protected:
	virtual void NativeConstruct() override;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> AskQuizText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ConfirmBtn;
	UFUNCTION()
	void OnConfirmQuiz();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseButton> CancelBtn;
	UFUNCTION()
	void OnCancelQuiz();
	UPROPERTY()
	TObjectPtr<class UOXQuizUI> OXQuizUI;
public:
	void InitParentUI(UOXQuizUI* Parent);
	void SetAskQuizText(const FText& DifficultyText);
	
protected:
	UPROPERTY()
	TObjectPtr<class AOXQuizTriggerActor> QuizTriggerActor;
	UPROPERTY()
	TObjectPtr<class ACuteAlienController> PC;
public:
	void SetQuizTriggerActor(AOXQuizTriggerActor* OXQuizTriggerActor);
	void SetPlayerController(ACuteAlienController* PlayerController);
	
};
