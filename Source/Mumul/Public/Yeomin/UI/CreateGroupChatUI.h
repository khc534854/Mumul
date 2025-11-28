// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateGroupChatUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UCreateGroupChatUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	UFUNCTION()
	void RefreshPlayerList();
	
	UPROPERTY()
	TObjectPtr<class AMumulGameState> GS;
	
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UGroupProfileUI> GroupProfileUIClass;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UGroupIconUI> GroupIconUIClass;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableTextBox> SearchBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> PlayerScrollBox;
	
	UPROPERTY()
	TObjectPtr<class UGroupChatUI> ParentUI;
public:
	void InitParentUI(UGroupChatUI* Parent);
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> CreateGroupBtn;
	UFUNCTION()
	void CreateGroupChat();
};
