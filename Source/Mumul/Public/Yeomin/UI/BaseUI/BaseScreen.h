// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseScreen.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UBaseScreen : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBorder> BackgroundBorder;

	// Background Color
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Content Background")
	FLinearColor BackgroundColor = FLinearColor::Black;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UNamedSlot> ContentSlot;
};
