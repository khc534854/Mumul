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

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> LogOutYesBtn; // "예"
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseButton> LogOutNoBtn;  // "아니오"

	UPROPERTY()
	TObjectPtr<class ACuteAlienController> PC;
};
