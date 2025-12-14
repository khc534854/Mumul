// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/DifficultyBubbleUI.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UDifficultyBubbleUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	InteractionBtn->OnHovered.AddDynamic(this, &UDifficultyBubbleUI::OnHoveredBorderColor);
	InteractionBtn->OnUnhovered.AddDynamic(this, &UDifficultyBubbleUI::OnUnhoveredBorderColor);
}

void UDifficultyBubbleUI::OnHoveredBorderColor()
{
	DifficultyBorder->SetBrushColor(FLinearColor::Green);
}

void UDifficultyBubbleUI::OnUnhoveredBorderColor()
{
	DifficultyBorder->SetBrushColor(FLinearColor::White);
}

void UDifficultyBubbleUI::SetDifficultyText(const FText& Text)
{
	DifficultyText->SetText(Text);
}
