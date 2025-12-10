// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CustomItemEntryUI.h"

#include "Components/CheckBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCustomItemEntryUI::InitItem(FName ItemID, UTexture2D* Thumbnail, FString ItemName,
                                    class ACuteAlienPlayer* Player)
{
	CosmeticItemID = ItemID;
	OwningPlayerCharacter = Player;

	if (ThumbnailImage && Thumbnail)
	{
		ThumbnailImage->SetBrushFromTexture(Thumbnail);
	}
	if (ItemNameText)
	{
		ItemNameText->SetText(FText::FromString(ItemName));
	}
}

void UCustomItemEntryUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ItemCheckBox)
	{
		ItemCheckBox->OnCheckStateChanged.AddDynamic(this, &UCustomItemEntryUI::OnCheckBoxStateChanged);
	}
}

void UCustomItemEntryUI::OnCheckBoxStateChanged(bool bIsChecked)
{
	OnItemChecked.Broadcast(CosmeticItemID, bIsChecked);
}
