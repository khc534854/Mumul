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
		ThumbnailImage->SetColorAndOpacity(FLinearColor(0.99f, 0.99f, 0.99f, 1.0f));
	}
    
	if (ItemNameText)
	{
		// [신규] 원래 이름 저장
		OriginalName = FText::FromString(ItemName);
		ItemNameText->SetVisibility(ESlateVisibility::Hidden);
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
	// 방어 코드: 필수 위젯이 없으면 중단
    if (!ItemNameText || !ItemCheckBox || !ThumbnailImage) return;

    bool bIsChecked = ItemCheckBox->IsChecked();

    // -------------------------------------------------------
    // 1. 공통 초기화 (기본 상태: 이름 숨김, 이미지 원래 색)
    // -------------------------------------------------------
    ItemNameText->SetVisibility(ESlateVisibility::Hidden);
    ThumbnailImage->SetColorAndOpacity(FLinearColor(0.99f, 0.99f, 0.99f, 1.0f));

    // -------------------------------------------------------
    // 2. 타입별 분기 처리
    // -------------------------------------------------------

    // [Type 1] 커스텀 아이템 (Custom)
    if (EntryType == EItemEntryType::Custom)
    {
       if (bIsChecked)
       {
          // [착용 상태]
          // 1. 텍스트 표시: "착용 해제"
          ItemNameText->SetVisibility(ESlateVisibility::Visible);
          ItemNameText->SetText(FText::FromString(TEXT("착용\n해제")));
          ItemNameText->SetColorAndOpacity(FLinearColor::White); 

          // 2. 이미지 톤 다운 (회색)
          ThumbnailImage->SetColorAndOpacity(FLinearColor(0.34f, 0.34f, 0.34f, 1.0f));
       }
       else
       {
          // [미착용 상태]
          // 텍스트 숨김 (공통 초기화에서 처리됨)
          // 이미지 원래 색 (공통 초기화에서 처리됨)
       }
    }
    // [Type 2] 하우징 아이템 (Housing)
    else if (EntryType == EItemEntryType::Housing)
    {
       if (bIsChecked)
       {
          // [설치 중 (프리뷰) 상태]
          // 1. 텍스트 표시: "배치 중"
          ItemNameText->SetVisibility(ESlateVisibility::Visible);
          ItemNameText->SetText(FText::FromString(TEXT("배치 중")));
          ItemNameText->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 0.0f, 1.0f)); // 강조색(노랑) 유지
          
          // 이미지는 톤 다운 하지 않음 (선택 중이므로 잘 보여야 함)
       }
       else
       {
          if (bIsPlaced)
          {
             // [설치 완료 상태]
             // 1. 텍스트 표시: "배치 됨"
             ItemNameText->SetVisibility(ESlateVisibility::Visible);
             ItemNameText->SetText(FText::FromString(TEXT("배치 됨")));
             ItemNameText->SetColorAndOpacity(FLinearColor::White);

             // 2. 이미지 톤 다운 (회색) -> 이미 설치했으므로 비활성 느낌
             ThumbnailImage->SetColorAndOpacity(FLinearColor(0.34f, 0.34f, 0.34f, 1.0f));
          }
          else
          {
             // [미설치 상태]
             // 텍스트 숨김 & 이미지 원래 색 (공통 초기화에서 처리됨)
          }
       }
    }
}
