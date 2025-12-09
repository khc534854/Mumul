// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/VoiceChatComponent.h"
#include "PlayerUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UPlayerUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnLogOutBtnClicked();
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> CurrentTime;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> Minimap;

	FTimerHandle MinimapBindTimer;
    
	// [신규] 미니맵 연결 함수
	UFUNCTION()
	void TryBindMinimap();
	
	FTimerHandle FirstMinuteTimer;
	UFUNCTION()
	void StartMinuteTimer();
	FTimerHandle TimeUpdater;
	UFUNCTION()
	void UpdateCurrentTime();

	UFUNCTION(BlueprintCallable)
	UVoiceChatComponent* GetVoiceComponent() const;
	UFUNCTION(BlueprintImplementableEvent)
	void ChangeMicStateImage();

	UPROPERTY()
	TObjectPtr<class ACuteAlienController> PC;
	UPROPERTY()
	TObjectPtr<class UGroupChatUI> GroupChatUI;
	FTimerHandle GroupChatCheckTimer;
	void CheckGroupChatUI();
public:
	void InitGroupChatUI(UGroupChatUI* UI);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> TentBtn;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> MicrophoneBtn;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation > MicOn;

protected:
	// 버튼 스타일 업데이트 함수 분리
	UFUNCTION()
	void UpdateMicButtonState(bool bActive);
	UFUNCTION()
	void UpdateRecordButtonState(bool bActive);

	UFUNCTION()
	void OnTentClicked();

	UFUNCTION()
	void OnMicClicked();
	UFUNCTION()
	void OnRecordClicked();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton > ProfileBtn;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton > LogOutBtn;
	bool bIsMainHovered = false;
	bool bIsSubHovered = false;
	bool bIsTryingToHide = false;
	FTimerHandle HideLogOutTimer;
	void TryHideLogOutBtn();
	void HideLogOutBtn();
	UFUNCTION()
	void OnProfileBtnHovered();
	UFUNCTION()
	void OnProfileBtnUnhovered();
	UFUNCTION()
	void OnLogOutBtnHovered();
	UFUNCTION()
	void OnLogOutBtnUnhovered();
};
