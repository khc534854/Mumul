// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LogoutUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API ULogoutUI : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnClickedLogoutYesBtn();
	UFUNCTION()
	void OnClickedLogoutNoBtn();
	UFUNCTION()
	void OnLogoutResponseReceived(bool bSuccess, FString Message);
	
	UFUNCTION()
	void OnClickedNextBtn();

public:
	UFUNCTION()
	void OpenHelpPopup();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> WidgetSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBaseButton> NextBtn;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> HelpImg;

	UPROPERTY(EditDefaultsOnly)
	TArray<class UTexture2D*> HelpImages;

	int32 CurImageIdx = 0;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> LogOutYesBtn; // "예"
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseButton> LogOutNoBtn;  // "아니오"

	UPROPERTY()
	TObjectPtr<class ACuteAlienController> PC;
};
