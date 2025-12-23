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

	// Minimap
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> Minimap;

	FTimerHandle MinimapBindTimer;
    
	UFUNCTION()
	void TryBindMinimap();
	
	// Customize
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> PlayerCustomizeBtn;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScaleBox> CustomizeBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UHorizontalBox> PlayerCustomizeItemBox;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation > CustomizeBoxAnim;

	bool bIsOpenCustomizeUI = false;

	UPROPERTY(EditDefaultsOnly, Category="Cosmetic")
	TSubclassOf<class UCustomItemEntryUI> ItemEntryUIClass;
    
	UPROPERTY(EditDefaultsOnly, Category="Cosmetic")
	TObjectPtr<class UDataTable> CustomItemDataTable; 

	UFUNCTION()
	void OnCustomizeBoxClick();
	void LoadAndGenerateItemList();
	UFUNCTION()
	void OnCustomItemEntryChecked(FName ItemID, bool bIsChecked);
    
	UPROPERTY()
	TMap<FName, TObjectPtr<UCustomItemEntryUI>> ItemWidgetMap;

	// Housing
protected:
	UFUNCTION()
	void OnTentClicked();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> TentBtn;

	UFUNCTION()
	void OnDeleteButtonClicked();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> HousingDeleteModeBtn;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> HousingBtn;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScaleBox> HousingBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UHorizontalBox> HousingItemBox;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation > HousingBoxAnim;

	bool bIsOpenHousingUI = false;
	
	UPROPERTY(EditDefaultsOnly, Category="Housing")
	TSubclassOf<class UCustomItemEntryUI> HousingItemEntryUIClass;
    
	// 데이터 테이블 레퍼런스 (에디터에서 연결)
	UPROPERTY(EditDefaultsOnly, Category="Housing")
	TObjectPtr<class UDataTable> HousingItemDataTable;
	
	UFUNCTION()
	void OnHousingBoxClick();
	void LoadAndGenerateHousingItemList();
	UFUNCTION()
	void OnHousingItemEntryChecked(FName ItemID, bool bIsChecked);


	UPROPERTY()
	TMap<FName, TObjectPtr<UCustomItemEntryUI>> HousingWidgetMap;

	// LogOut
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton > ProfileBtn;
public:
	void SetProfileBtnIMG(UTexture2D* IMG);
	void CloseSidePanels();
	

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton > LogOutBtn;
	bool bIsMainHovered = false;
	bool bIsSubHovered = false;
	bool bIsTryingToHide = false;
	FTimerHandle HideLogOutTimer;

	UFUNCTION()
	void OnLogOutBtnClicked();
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

	// Voice
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> MicrophoneBtn;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<class UWidgetAnimation > MicOn;

	UFUNCTION(BlueprintCallable)
	UVoiceChatComponent* GetVoiceComponent() const;
	UFUNCTION(BlueprintImplementableEvent)
	void ChangeMicStateImage();
	UFUNCTION()
	void UpdateMicButtonState(bool bActive);
	UFUNCTION()
	void UpdateRecordButtonState(bool bActive);
	UFUNCTION()
	void OnMicClicked();
	UFUNCTION()
	void OnRecordClicked();

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> NoticeBtn;
	UFUNCTION()
	void OnClickNoticeBtn();
	
	// Time
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBaseText> CurrentTime;
	
	FTimerHandle FirstMinuteTimer;
	FTimerHandle TimeUpdater;
	
	UFUNCTION()
	void StartMinuteTimer();
	UFUNCTION()
	void UpdateCurrentTime();

public :
	// 공용 변수
	UPROPERTY()
	TObjectPtr<class ACuteAlienController> PC;
	UPROPERTY()
	TObjectPtr<class UGroupChatUI> GroupChatUI;
	
	FTimerHandle GroupChatCheckTimer;
	
	void CheckGroupChatUI();
	void ResetHousingSelection();
	void InitGroupChatUI(UGroupChatUI* UI);

	void MarkHousingItemAsPlaced(FName ItemID, bool bPlaced);

	void CheckEquippedCustomItem();
	void CheckPlacedHousingItems();

	void CancelTent();
	
protected:
	UPROPERTY()
	TMap<UButton*, FButtonStyle> OriginalButtonStyles;

	// [신규] 버튼의 활성화 상태에 따라 스타일을 교체하는 함수
	void SetButtonActiveState(UButton* TargetBtn, bool bIsActive);

	// [신규] 모든 메뉴 버튼의 시각적 상태를 초기화(OFF)하는 함수
	void ResetAllMenuButtons();

	

protected:
	// UI 조작이 가능한지 확인하는 플래그
	bool bIsUIBusy = false;

	// 디바운싱용 타이머 핸들
	FTimerHandle DebounceTimerHandle;

	// 잠금을 해제하는 함수
	void UnlockUIInteraction() { bIsUIBusy = false; }
    
	// UI 잠금을 거는 헬퍼 함수 (시간 지정 가능)
	bool TryLockUI(float Duration = 0.3f);
};
