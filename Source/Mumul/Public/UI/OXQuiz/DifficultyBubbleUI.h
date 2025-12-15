// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DifficultyBubbleUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UDifficultyBubbleUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	bool bIsHovered = false;
	FTimerHandle UnhoverTimer;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> InteractionBtn;
	void UpdateBorderColor();
	UFUNCTION()
	void OnHoveredBorderColor();
	void ApplyUnhover();
	UFUNCTION()
	void OnUnhoveredBorderColor();
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UBorder> DifficultyBorder;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> DifficultyText;
	
public:
	void SetDifficultyText(const FText& Difficulty);
};
