// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DispatchPopUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UDispatchPopUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY()
	TObjectPtr<class ACuteAlienController> PC;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> DispatchBtn;
	UFUNCTION()
	void OnClickPopUp();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> DispatchText;
	FString MakePreviewText(const FString& OriginalText, int32 MaxLength);
	
public:
	void SetDispatchText(const FString& Text);
};
