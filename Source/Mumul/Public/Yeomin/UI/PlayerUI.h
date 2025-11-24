// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UPlayerUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY()
	TObjectPtr<class ACuteAlienController> PC;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> TentBtn;
	UFUNCTION()
	void OnTentClicked();
	
};
