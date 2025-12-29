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
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> DispatchText;
	FString MakePreviewText(const FString& OriginalText, int32 MaxLength);
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TimeStampText;
	
public:
	void SetDispatchText(const FString& Text);
	void SetTimeStampText(const FDateTime& Text);
};
