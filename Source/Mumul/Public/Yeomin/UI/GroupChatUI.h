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
	
	UFUNCTION()
	void OnTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> ChatScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableTextBox> EditBox;
	
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatMessageBlockUI> ChatMessageBlockUIClass;
	void AddChat(FString Text);
	
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatMessageBlockUI> GroupProfileUIClass;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> AddGroupBtn;
	UFUNCTION()
	void ShowInvitationUI();
	
	
};
