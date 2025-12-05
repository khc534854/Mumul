// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "khc/Player/VoiceChatComponent.h"
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
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> RecordBtn;
	
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
};
