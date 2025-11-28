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
	UPROPERTY()
	TObjectPtr<class UGroupChatUI> ParentUI;
public:
	void InitParentUI(class UGroupChatUI* Parent);
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> GroupIconBtn;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatBlockUI> ChatBlockUIClass;
	UPROPERTY()
	TObjectPtr<class UChatBlockUI> GroupChatUI;
	TArray<FString> PlayersInGroup;
};
