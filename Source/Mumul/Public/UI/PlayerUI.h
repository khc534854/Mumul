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
};
