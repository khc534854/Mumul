// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/CustomItemEntryUI.h"

#include "Components/CheckBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCustomItemEntryUI::InitItem(FName ItemID, UTexture2D* Thumbnail, FString ItemName,
                                    class ACuteAlienPlayer* Player,  EItemEntryType Type)
{
	CosmeticItemID = ItemID;
	OwningPlayerCharacter = Player;
	EntryType = Type; // [신규] 타입 저장

	if (ThumbnailImage && Thumbnail)
	{
		ThumbnailImage->SetBrushFromTexture(Thumbnail);
	}
    
	if (ItemNameText)
	{
		// [신규] 원래 이름 저장
		OriginalName = FText::FromString(ItemName);
		ItemNameText->SetText(OriginalName);
       
		// 초기화 시 기본 색상(흰색) 설정
		ItemNameText->SetColorAndOpacity(FLinearColor::White);
	}

	// 초기 상태 업데이트
	UpdateVisualState();
}

void UCustomItemEntryUI::SetPlacedState(bool bPlaced)
{
	bIsPlaced = bPlaced;
	UpdateVisualState();
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

	UpdateVisualState();
}

void UCustomItemEntryUI::SetItemCheckState(bool bNewState)
{
	if (ItemCheckBox)
	{
		ItemCheckBox->SetIsChecked(bNewState);
		UpdateVisualState();
	}
}

void UCustomItemEntryUI::UpdateVisualState()
{
	if (!ItemNameText || !ItemCheckBox) return;

	bool bIsChecked = ItemCheckBox->IsChecked();

	// 1. 커스텀 아이템 (Custom)
	if (EntryType == EItemEntryType::Custom)
	{
		if (bIsChecked)
		{
			// 착용 (체크 ON) -> "착용중" (초록)
			ItemNameText->SetText(FText::FromString(TEXT("착용중")));
			ItemNameText->SetColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f, 1.0f));
		}
		else
		{
			// 미착용 (체크 OFF) -> 원래 이름 (흰색)
			ItemNameText->SetText(OriginalName);
			ItemNameText->SetColorAndOpacity(FLinearColor::White);
		}
	}
	// 2. 하우징 아이템 (Housing)
	else if (EntryType == EItemEntryType::Housing)
	{
		if (bIsChecked)
		{
			// 배치 시도 중 (체크 ON) -> "배치중" (노랑)
			ItemNameText->SetText(FText::FromString(TEXT("배치중")));
			ItemNameText->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow
		}
		else
		{
			if (bIsPlaced)
			{
				// 설치 완료 상태 (체크 OFF + bIsPlaced true) -> "배치됨" (초록)
				ItemNameText->SetText(FText::FromString(TEXT("배치됨")));
				ItemNameText->SetColorAndOpacity(FLinearColor(0.0f, 0.5f, 1.0f, 1.0f));
			}
			else
			{
				// 미선택/기본 (체크 OFF) -> 원래 이름 (흰색)
				ItemNameText->SetText(OriginalName);
				ItemNameText->SetColorAndOpacity(FLinearColor::White);
			}
		}
	}
}
