// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/DifficultyBubbleUI.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UDifficultyBubbleUI::NativeConstruct()
{
	Super::NativeConstruct();

	bIsHovered = false;
	UpdateBorderColor();

	InteractionBtn->OnHovered.AddDynamic(this, &UDifficultyBubbleUI::OnHoveredBorderColor);
	InteractionBtn->OnUnhovered.AddDynamic(this, &UDifficultyBubbleUI::OnUnhoveredBorderColor);
}

void UDifficultyBubbleUI::UpdateBorderColor()
{
	DifficultyBorder->SetBrushColor(bIsHovered ? FLinearColor::Green : FLinearColor::White);
}

void UDifficultyBubbleUI::OnHoveredBorderColor()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(UnhoverTimer);
	}
	bIsHovered = true;
	UpdateBorderColor();
}

void UDifficultyBubbleUI::ApplyUnhover()
{
	if (bIsHovered)
	{
		bIsHovered = false;
		UpdateBorderColor();
	}
}

void UDifficultyBubbleUI::OnUnhoveredBorderColor()
{
	GetWorld()->GetTimerManager().SetTimer(
		UnhoverTimer,
		this,
		&UDifficultyBubbleUI::ApplyUnhover,
		0.1f,
		false
	);
	
}

void UDifficultyBubbleUI::SetDifficultyText(const FText& Text)
{
	DifficultyText->SetText(Text);
}
