// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseTextBox.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UBaseTextBox : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UEditableTextBox> BaseTextBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Content")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Content")
	FText HintText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Content")
	int32 FontSize = 24;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Content")
	FSlateFontInfo BaseFont;
};