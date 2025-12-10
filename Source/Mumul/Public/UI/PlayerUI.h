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
	TObjectPtr<class UButton> PlayerCustomizeBtn;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScaleBox> CustomizeBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UHorizontalBox> PlayerCustomizeItemBox;

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

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation > CustomizeBoxAnim;

	bool bIsOpenCustomizeUI = false;

	UFUNCTION()
	void OnCustomizeBoxClick();
	void LoadAndGenerateItemList();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Cosmetic")
	TSubclassOf<class UCustomItemEntryUI> ItemEntryUIClass; // 새로 만든 위젯 클래스
    
	// 데이터 테이블 레퍼런스 (에디터에서 연결)
	UPROPERTY(EditDefaultsOnly, Category="Cosmetic")
	TObjectPtr<class UDataTable> CustomItemDataTable; 

	// 장착 상태 변경 핸들러
	UFUNCTION()
	void OnCustomItemEntryChecked(FName ItemID, bool bIsChecked);
    
	// 장착된 아이템의 ID를 저장 (선택된 위젯을 빠르게 찾기 위해)
	UPROPERTY()
	TMap<FName, TObjectPtr<UCustomItemEntryUI>> ItemWidgetMap;
	
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
