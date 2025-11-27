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
	UVoiceChatComponent* GetVoiceComponent() const;

	UPROPERTY()
	TObjectPtr<class ACuteAlienController> PC;
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> TentBtn;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> MicrophoneBtn;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> RecordBtn;
	
	UFUNCTION()
	void OnTentClicked();

	UFUNCTION()
	void OnMicClicked();
	UFUNCTION()
	void OnRecordClicked();


public:
	UPROPERTY(BlueprintReadOnly)
	bool bIsSpeaking = false;
	UPROPERTY(BlueprintReadOnly)
	bool bIsRecoding = false;
	
};
