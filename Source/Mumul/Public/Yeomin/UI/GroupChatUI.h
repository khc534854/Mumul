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
	
	UPROPERTY()
	TObjectPtr<class UHttpNetworkSubsystem> HttpSystem;
	
	void ToggleVisibility(UWidget* Widget);

	// [신규] 웹소켓 서브시스템
	UPROPERTY()
	TObjectPtr<class UWebSocketSubsystem> WebSocketSystem;

	// [신규] 현재 보고 있는 채팅방 (이전 방과 비교하기 위해 저장)
	UPROPERTY()
	TObjectPtr<class UGroupIconUI> CurrentSelectedGroup;

public:
	// [신규] 그룹 아이콘이 클릭되었을 때 호출되는 함수 (방 전환 메인 로직)
	void SelectGroupChat(class UGroupIconUI* SelectedIcon);

protected:
	// [신규] 챗봇 응답 핸들러
	UFUNCTION()
	void OnAIChatStarted(FString Message);
    
	UFUNCTION()
	void OnAIChatAnswer(FString Answer);
    
	// [신규] 챗봇 방 초기화 함수
	void InitChatbotRoom();

	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatBlockUI> ChatBlockUIClass;
	
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatMessageBlockUI> BotChatMessageBlockUIClass;

public:
	// [신규] 챗봇 메시지(또는 경고문)를 화면에 추가하는 함수
	void AddBotChat(const FString& Message);
public:
	void AddChatBlock(class UChatBlockUI* UI) const;
	void RemoveChatBlock() const;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableTextBox> EditBox;
	UFUNCTION()
	void OnTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
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
	TObjectPtr<class UTextBlock> GroupNameTitle; 
	
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
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ToggleVisibilityBtn;
	UFUNCTION()
	void ToggleInvitationUI();
	UFUNCTION()
	void OnToggleVisibilityBtn();
};
