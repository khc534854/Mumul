// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseExitButton.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UBaseExitButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativePreConstruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> BaseExitButton;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Content Button")
	TObjectPtr<UTexture2D> ButtonImage;

	// 정사각형 사이즈 하나만 조정
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Content Button")
	float ImageSize = 128.f;
};
