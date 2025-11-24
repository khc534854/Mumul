// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/PlayerUI.h"

#include "Components/Button.h"
#include "Yeomin/Player/CuteAlienController.h"

void UPlayerUI::NativeConstruct()
{
	Super::NativeConstruct();

	TentBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnTentClicked);

	PC = Cast<ACuteAlienController>(GetOwningPlayer());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("RadialUI Can't Find PlayerController"))
	}
}

void UPlayerUI::OnTentClicked()
{
	PC->ShowPreviewTent();
}
