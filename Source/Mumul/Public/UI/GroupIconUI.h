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
	UPROPERTY()
	TObjectPtr<class UIMGManager> IMGManager;

	UPROPERTY(meta = (BindWidget))
	class UImage* NewMessageNotice;

	UPROPERTY(meta = (BindWidget))
	class UImage* MeetingActiveNotice;
	
	UPROPERTY()
	TObjectPtr<class UHttpNetworkSubsystem> HttpSystem;
	//UFUNCTION()
	//void OnServerTeamChatMessageResponse(bool bSuccess, FString Message);
	UFUNCTION()
	void DisplayGroupChat();
	UPROPERTY()
	TObjectPtr<class UGroupChatUI> ParentUI;
public:
	void InitParentUI(class UGroupChatUI* Parent);

	void SetHighlight(bool bIsFocus);

	void SetNewMessageNotice(bool bVisible);

	void SetMeetingStatus(bool bIsActive);
protected:
	UPROPERTY(meta=(BindWidget))               
	TObjectPtr<class UBorder> BackgroundBorder;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> GroupIconBtn;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatBlockUI> ChatBlockUIClass;
	UPROPERTY(EditDefaultsOnly, Category="UI Class")
	TSubclassOf<class UChatMessageBlockUI> ChatMessageBlockUIClass;
	
public:
	void SetIconIMG(UTexture2D* IMG);
	UPROPERTY()
	TObjectPtr<class UChatBlockUI> ChatBlockUI;

public:
	// [신규] 이 아이콘이 챗봇 방인지 여부
	bool bIsChatbotRoom = false;

	UPROPERTY()
	TObjectPtr<class UAudioManager> AudioManager;
};
