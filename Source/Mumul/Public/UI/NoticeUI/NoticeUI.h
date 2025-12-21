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
	int64 NoticeId;          // 공지 고유 ID

	UPROPERTY()
	FString Content;

	UPROPERTY()
	FDateTime CreatedAt;
};

USTRUCT(BlueprintType)
struct FUserNoticeState
{
	GENERATED_BODY()

	UPROPERTY()
	int64 NoticeId;

	UPROPERTY()
	FString UserId;

	UPROPERTY()
	bool bConfirmed = false;

	UPROPERTY()
	FDateTime ConfirmedAt;
};

USTRUCT()
struct FNoticeViewData
{
	GENERATED_BODY()

	UPROPERTY()
	FNoticeData Notice;

	UPROPERTY()
	FUserNoticeState UserState;
};

UCLASS()
class MUMUL_API UNoticeUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBorder> NoticeBorder;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> NoticeTap;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> DirectMessageTap;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UWidgetSwitcher> NoticeWS;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> NoticeScrollBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UVerticalBox> UnConfirmedVBox;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UVerticalBox> ConfirmedVBox;
	UPROPERTY()
	TObjectPtr<class UNoticeContentUI> NoticeContentUI;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ToggleNoticeBtn;
	UFUNCTION()
	void OnToggleNoticeVisibility();
	
public:
	void AddNotice(const FNoticeViewData& Data);
	void SortNotices();
};
