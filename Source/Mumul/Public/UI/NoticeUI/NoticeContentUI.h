// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NoticeContentUI.generated.h"

struct FNoticeViewData;
/**
 * 
 */
UCLASS()
class MUMUL_API UNoticeContentUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> NoticeContentText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> NoticeConfirmBtn;
	UFUNCTION()
	void OnConfirmClicked();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> NoticeConfirmText;
	
	int32 NoticeId;
	FDateTime CreatedAt;
	bool bIsConfirmed = false;
	
public:
	void InitUI(const FNoticeViewData& Data);
	FDateTime GetCreatedAt() const { return CreatedAt; }
	bool IsConfirmed() const { return bIsConfirmed; }
	void UpdateConfirmButtonUI();
};
