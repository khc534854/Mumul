// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InvitationUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UInvitationUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	UFUNCTION()
	void RefreshJoinedPlayerList();
	
	UPROPERTY()
	TObjectPtr<class AMumulGameState> GS;
	
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UGroupProfileUI> GroupProfileUIClass;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> PlayerScrollBox;
};
