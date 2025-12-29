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
	
public:
	void SetDispatchText(const FString& Text);
};
