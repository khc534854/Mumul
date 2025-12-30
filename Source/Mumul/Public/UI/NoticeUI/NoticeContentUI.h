// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Network/NetworkStructs.h"
#include "NoticeContentUI.generated.h"


/**
 * 
 */
UCLASS()
class MUMUL_API UNoticeContentUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY()
	TObjectPtr<class UWebSocketSubsystem> WebSocketSystem;
	UFUNCTION()
	void OnAckResponse(const FDispatchAckPongPayload& AckPong);
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> NoticeContentText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> NoticeConfirmBtn;
	UFUNCTION()
	void OnConfirmClicked();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> NoticeConfirmText;
	
	int32 ContentID;
	FDateTime CreatedAt;
	bool bIsConfirmed = false;
	
	bool bIsNotice = true;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TimeStampText;
	
public:
	void InitUI(const FDispatchPayloadBase& Data);
	FDateTime GetCreatedAt() const { return CreatedAt; }
	int32 GetMessageId() const { return ContentID; }
	bool IsConfirmed() const { return bIsConfirmed; }
	void UpdateConfirmButtonUI();
	void SetTimeStampText(const FDateTime& Text);
};
