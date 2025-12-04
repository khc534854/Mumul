// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseText.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UBaseText : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativePreConstruct() override;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> BaseText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Content")
	FText Text = FText::GetEmpty();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Content")
	int32 FontSize = 24;

	// Base Font
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Content")
	FSlateFontInfo BaseFont;
};
