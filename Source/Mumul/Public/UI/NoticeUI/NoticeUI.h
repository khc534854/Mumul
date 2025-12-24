// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NoticeUI.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FNoticeData
{
	GENERATED_BODY()

	UPROPERTY()
	FString noticeId;

	UPROPERTY()
	FString title;
	
	UPROPERTY()
	FString text;

	UPROPERTY()
	FDateTime CreatedAt;
	
	UPROPERTY()
	bool bConfirmed = false;
};

USTRUCT()
struct FDMData
{
	GENERATED_BODY()

	UPROPERTY()
	FString messageId;

	UPROPERTY()
	FString text;
	
	UPROPERTY()
	FDateTime CreatedAt;
	
	UPROPERTY()
	bool bConfirmed = false;
};

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
	
	ENoticeState CurNoticeState = ENoticeState::Notice;
	
	void ChangeNoticeState(ENoticeState NewState);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBorder> NoticeBorder;
	
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
	
public:
	void OnToggleNoticeVisibility();
	void AddNotice(const FNoticeData& Data);
	void AddDM(const FDMData& Data);
};
