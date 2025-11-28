// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GroupChatUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UGroupChatUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> ChatSizeBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableTextBox> EditBox;
	UFUNCTION()
	void OnTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatMessageBlockUI> ChatMessageBlockUIClass;
	void AddChat(FString Text);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> AddGroupBtn; 
	
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UCreateGroupChatUI> CreateGroupChatUIClass;
	UPROPERTY()
	TObjectPtr<class UCreateGroupChatUI> CreateGroupChatUI;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> CreateGroupChatBox;
	UFUNCTION()
	void ShowCreateGroupChatUI();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> GroupScrollBox;
public:
	void AddGroupIcon(class UGroupIconUI* UI);
	
};
