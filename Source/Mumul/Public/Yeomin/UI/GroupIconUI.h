// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GroupIconUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UGroupIconUI : public UUserWidget
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;
	
protected:
	UFUNCTION()
	void DisplayGroupChat();
	
public:
	UPROPERTY()
	TObjectPtr<class UButton> GroupIconBtn;
	TArray<FString> PlayersInGroup;
};
