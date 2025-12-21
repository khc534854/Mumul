// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeUI/NoticeUI.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Data/ObjectAndClassFinder.h"
#include "UI/NoticeUI/NoticeContentUI.h"

void UNoticeUI::NativeConstruct()
{
	Super::NativeConstruct();

	ToggleNoticeBtn->OnClicked.AddDynamic(this, &UNoticeUI::OnToggleNoticeVisibility);
	NoticeBorder->SetVisibility(ESlateVisibility::Collapsed);
}

void UNoticeUI::OnToggleNoticeVisibility()
{
	if (NoticeBorder->GetVisibility() == ESlateVisibility::Collapsed)
	{
		SortNotices();
		NoticeBorder->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		NoticeBorder->SetVisibility(ESlateVisibility::Collapsed);
	}
	
	// Test Add Notice
	FNoticeViewData NoticeData;
	NoticeData.Notice.Content = FString(TEXT("Test Notice!"));
	NoticeData.Notice.CreatedAt = FDateTime::Now();
	NoticeData.UserState.bConfirmed = false;
	AddNotice(NoticeData);
}

void UNoticeUI::SortNotices()
{
	TArray<UNoticeContentUI*> AllNoticeUIs;

	// 1️⃣ 모든 공지 아이템 수집
	for (UWidget* Child : UnConfirmedVBox->GetAllChildren())
	{
		if (UNoticeContentUI* NoticeUI = Cast<UNoticeContentUI>(Child))
		{
			AllNoticeUIs.Add(NoticeUI);
		}
	}

	for (UWidget* Child : ConfirmedVBox->GetAllChildren())
	{
		if (UNoticeContentUI* NoticeUI = Cast<UNoticeContentUI>(Child))
		{
			AllNoticeUIs.Add(NoticeUI);
		}
	}

	// 2️⃣ 상태 + 시간 기준 정렬
	AllNoticeUIs.Sort([](const UNoticeContentUI& First, const UNoticeContentUI& Second)
	{
		// 1차: 미확인 공지 우선
		if (First.IsConfirmed() != Second.IsConfirmed())
		{
			return !First.IsConfirmed();
		}

		// 2차: 생성 시간 내림차순 (최근 공지 우선)
		return First.GetCreatedAt() > Second.GetCreatedAt();
	});

	// 3️⃣ 정렬 결과를 UI에 반영
	UnConfirmedVBox->ClearChildren();
	ConfirmedVBox->ClearChildren();

	for (UNoticeContentUI* NoticeUI : AllNoticeUIs)
	{
		if (NoticeUI->IsConfirmed())
		{
			ConfirmedVBox->AddChild(NoticeUI);
		}
		else
		{
			UnConfirmedVBox->AddChild(NoticeUI);
		}
	}
}

void UNoticeUI::AddNotice(const FNoticeViewData& Data)
{
	if (TSubclassOf<UNoticeContentUI> UNoticeContentUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<
		UNoticeContentUI>("WBP_NoticeContent"))
	{
		NoticeContentUI = CreateWidget<UNoticeContentUI>(this, UNoticeContentUIClass);
		NoticeContentUI->InitUI(Data);
		UnConfirmedVBox->InsertChildAt(0, NoticeContentUI);
	}
}
