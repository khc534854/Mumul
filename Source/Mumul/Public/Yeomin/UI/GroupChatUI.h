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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY()
	TObjectPtr<class UHttpNetworkSubsystem> HttpSystem;
	
	void ToggleVisibility(UWidget* Widget);

public:
	void AddChatBlock(class UChatBlockUI* UI) const;
	void RemoveChatBlock() const;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UMultiLineEditableTextBox> EditBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ChatEnter;
	UFUNCTION()
	void OnTextBoxCommitted();
	UFUNCTION()
	void OnServerChatMessageResponse(bool bSuccess, FString Message);
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatMessageBlockUI> ChatMessageBlockUIClass;
public:
	void AddChat(const FString& TeamID, const FString& CurrentTime, const FString& Name, const FString& Text) const;
		
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> ChatSizeBox;
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> AddGroupBtn; 
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> GroupNameTitle; 
	
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UCreateGroupChatUI> CreateGroupChatUIClass;
	UPROPERTY()
	TObjectPtr<class UCreateGroupChatUI> CreateGroupChatUI;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> CreateGroupChatBox;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UGroupIconUI> GroupIconUIClass;
	UFUNCTION()
	void OnServerTeamChatListResponse(bool bSuccess, FString Message);
public:
	void SetGroupNameTitle(const FString& GroupName);
	UFUNCTION()
	void ToggleCreateGroupChatUI();
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> GroupScrollBox;
public:
	void AddGroupIcon(class UGroupIconUI* UI) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UInvitationUI> InvitationUIClass;
	UPROPERTY()
	TObjectPtr<class UInvitationUI> InvitationUI;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> InvitationBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> InviteBtn;
	
	UPROPERTY(editDefaultsOnly, Category="UI Image")
	TObjectPtr<class UTexture2D> LeftIMG;
	UPROPERTY(editDefaultsOnly, Category="UI Image")
	TObjectPtr<class UTexture2D> RightIMG;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBorder> GroupChatBorder;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ToggleVisibilityBtn;
	float AlignmentVal = 0.178f;
	bool bIsToggled = false;
	UFUNCTION()
	void ToggleInvitationUI();
	UFUNCTION()
	void OnToggleVisibilityBtn();
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> RecordBtn;
};
