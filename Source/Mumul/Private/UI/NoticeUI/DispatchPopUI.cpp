// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeUI/DispatchPopUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/CuteAlienController.h"
#include "Player/Component/PlayerNoticeComponent.h"
#include "UI/PlayerUI.h"
#include "UI/NoticeUI/NoticeUI.h"


void UDispatchPopUI::NativeConstruct()
{
	Super::NativeConstruct();

	PC = Cast<ACuteAlienController>(GetOwningPlayer());

	DispatchBtn->OnClicked.AddDynamic(this, &UDispatchPopUI::OnClickPopUp);
}

void UDispatchPopUI::OnClickPopUp()
{
	if (PC->NoticeComp->NoticeUI->bIsNoticeVisible == false)
	{
		if (!PC->PlayerUI->TryLockUI(0.5f)) return;
		
		PC->PlayerUI->CloseAllSidePanels();

		// 공지 열기 (NoticeUI의 토글 함수 호출)
		if (PC && PC->NoticeComp && PC->NoticeComp->NoticeUI)
		{
			PC->NoticeComp->NoticeUI->OnToggleNoticeVisibility();
			PC->PlayerUI->bIsOpenNoticeUI = true;
			// SetButtonActiveState(NoticeBtn, true); // 버튼 스타일이 있다면
		}
		
		PC->PlayerUI->RefreshInputMode();
	}
	DispatchBtn->OnClicked.RemoveDynamic(this, &UDispatchPopUI::OnClickPopUp);
}

FString UDispatchPopUI::MakePreviewText(const FString& OriginalText, int32 MaxLength)
{
	if (OriginalText.Len() <= MaxLength)
	{
		return OriginalText;
	}
	return OriginalText.Left(MaxLength) + TEXT("…");
}

void UDispatchPopUI::SetDispatchText(const FString& Text)
{
	const int32 PreviewLength = 75; // 원하는 미리보기 길이
	FString PreviewText = MakePreviewText(Text, PreviewLength);

	DispatchText->SetText(FText::FromString(PreviewText));
}
