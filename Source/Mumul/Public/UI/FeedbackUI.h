// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FeedbackUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UFeedbackUI : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnClickedConfirmEndBtn();
	UFUNCTION()
	void OnClickedCancelEndBtn();
	UFUNCTION()
	void OnClickedFeedbackExitBtn();

	UFUNCTION()
	void OnFeedbackResponseReceived(bool bSuccess, FString Message);
public:
	void OpenFeedbackUI();
	// 피드백 화면
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UWidgetSwitcher> FeedbackWidgetSwitcher;
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<class UWidgetAnimation> Feedback_SlideAnim;
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<class UWidgetAnimation> FeedbackResult_SlideAnim;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseTextBox> FeedbackText; 

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ConfirmEndBtn; // "예"
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseButton> CancelEndBtn;  // "아니오"

	// 피드백 결과 화면
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> FeedbackResultText;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseButton> FeedbackExitBtn;  // "아니오"

	
};
