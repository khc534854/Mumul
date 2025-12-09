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
	TObjectPtr<class UIMGManager> IMGManager;
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
	// [신규] 학습 챗봇용 핸들러 (1:1)
	UFUNCTION()
	void OnLearningChatStarted(FString Message);
    
	UFUNCTION()
	void OnLearningChatAnswer(FString Answer);
    
	UFUNCTION()
	void OnLearningChatEnded(FString Message);

	// [신규] 회의 도우미용 핸들러 (그룹)
	UFUNCTION()
	void OnMeetingChatStarted(FString Message, FString GroupId, FString UserName);

	UFUNCTION()
	void OnMeetingChatAnswer(FString Answer, FString GroupId);

	UFUNCTION()
	void OnMeetingChatEnded(FString Message, FString GroupId);
    
	// [신규] 챗봇 방 초기화 함수
	void InitChatbotRoom();

	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatBlockUI> ChatBlockUIClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="UI Class")
	TSubclassOf<class UChatMessageBlockUI> BotChatMessageBlockUIClass;

	UFUNCTION()
	void OnServerChatHistoryResponse(bool bSuccess, FString Message);
    
	// 시간 파싱 헬퍼 (2025-12-05T... -> 10:25)
	FString ParseTimeFromISO8601(const FString& IsoString);

public:
	// [신규] 챗봇 메시지(또는 경고문)를 화면에 추가하는 함수
	void AddBotChat(const FString& Message);
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
	UPROPERTY()
	TObjectPtr<class UGroupIconUI> ChatbotIcon;
	UPROPERTY(EditDefaultsOnly, Category="UI Image")
	TObjectPtr<class UTexture2D> MumuLeeOnIMG;
	UPROPERTY(EditDefaultsOnly, Category="UI Image")
	TObjectPtr<class UTexture2D> MumuLeeOffIMG;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> MumuLeeBox;
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
	UFUNCTION()
	void ToggleInvitationUI();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ToggleVisibilityBtn;
	bool bIsToggled = false;
	float AlignmentVal = 0.1968f;
	float StartVal;
	float TargetVal;
	float Elapsed;
	UPROPERTY()
	float Duration = 0.963f;
	bool bAnimating = false;
	void ToggleGroupChatAlignment();
	
public:
	bool IsGroupChatToggled() const { return bIsToggled; }
	UFUNCTION()
	void OnToggleVisibilityBtn();
	UPROPERTY(EditAnywhere, Category="Testing")
	bool bITestCreateTeamChat = false;

	// [신규] AI 도우미 토글 버튼
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> QuestionBtn; // 체크박스로 구현 (On/Off)

	UFUNCTION()
	void OnAICheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnClickQuestionBtn();

private:
	void UpdateQuestionButtonState();
	// 현재 AI 도우미가 켜져 있는지 확인하는 플래그
	bool bIsMeetingChatbotActive = false;

	// 헬퍼: 현재 방 정보 가져오기
	FString GetCurrentTeamID() const;
	FString GetCurrentTeamName() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category="UI Image")
	TArray<TObjectPtr<class UTexture2D>> RecordIMGs;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> RecordIMG;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> RecordText0;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> RecordText1;
	FTimerHandle DotTimer;
	int32 DotCount = 0;
	void UpdateDot();
	UPROPERTY(EditDefaultsOnly, Category="UI Image")
	TObjectPtr<class UTexture2D> NaNumiOnIMG;
	UPROPERTY(EditDefaultsOnly, Category="UI Image")
	TObjectPtr<class UTexture2D> NaNumiOffIMG;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> NaNumiIMG;
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> RecordBtn;
	void OnRecordBtnState(bool bIsOn);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScaleBox> NaNumiScaleBox;
};
