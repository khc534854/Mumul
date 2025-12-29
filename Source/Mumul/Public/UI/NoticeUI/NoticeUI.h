// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NoticeUI.generated.h"

/**
 * 
 */

struct FDispatchPayloadBase;

UENUM()
enum class ENoticeState : uint8
{
	Notice,
	Information,
	DM,
};

UCLASS()
class MUMUL_API UNoticeUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	
	UPROPERTY()
	TObjectPtr<class UAudioManager> AudioManager;
	
	ENoticeState CurNoticeState = ENoticeState::Notice;
	
	void ChangeNoticeState(ENoticeState NewState);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBorder> NoticeBorder;
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<class UWidgetAnimation> NoticeUI_SildeUpAnim;

	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> NoticeTap;
	UFUNCTION()
	void OnSwitchToNotice();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> InformationTap;
	UFUNCTION()
	void OnSwitchToInformation();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> DirectMessageTap;
	UFUNCTION()
	void OnSwitchToDM();
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UWidgetSwitcher> NoticeWS;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> NoticeScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UVerticalBox> UnConfirmedVBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UVerticalBox> ConfirmedVBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> InformationScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> DMScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UVerticalBox> DMUnConfirmedVBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UVerticalBox> DMConfirmedVBox;
	UPROPERTY()
	TObjectPtr<class UNoticeContentUI> NoticeContentUI;
	
	void SortNotices(UVerticalBox* ConfirmedBox, UVerticalBox* UnConfirmedBox);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class USizeBox> DispatchBox;
	UPROPERTY()
	TObjectPtr<class UDispatchPopUI> DispatchPopUI;
	UPROPERTY(meta=(BindWidgetAnim), Transient)
	TObjectPtr<class UWidgetAnimation> DispatchBox_SlideAnim;
	
public:
	void OnToggleNoticeVisibility();
	void AddNotice(const FDispatchPayloadBase& Data);
	void AddDM(const FDispatchPayloadBase& Data);
	
	bool bIsNoticeVisible = false;
	
	void DisplayDispatchPopUp(const FDispatchPayloadBase& DispatchPayload);
	void HideDispatchPopUp();
};
