// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeUI/NoticeUI.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Data/ObjectAndClassFinder.h"
#include "Player/CuteAlienController.h"
#include "UI/PlayerUI.h"
#include "UI/NoticeUI/NoticeContentUI.h"

void UNoticeUI::NativeConstruct()
{
	Super::NativeConstruct();

	NoticeTap->OnClicked.AddDynamic(this, &UNoticeUI::OnSwitchToNotice);
	InformationTap->OnClicked.AddDynamic(this, &UNoticeUI::OnSwitchToInformation);
	DirectMessageTap->OnClicked.AddDynamic(this, &UNoticeUI::OnSwitchToDM);

	ChangeNoticeState(ENoticeState::Notice);
}

void UNoticeUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// Init NoticeUI position
	PlayAnimation(NoticeUI_SildeUpAnim, NoticeUI_SildeUpAnim->GetEndTime(), 1, EUMGSequencePlayMode::Reverse);
}

void UNoticeUI::ChangeNoticeState(ENoticeState NewState)
{
	CurNoticeState = NewState;

	switch (NewState)
	{
	case ENoticeState::Notice:
		NoticeWS->SetActiveWidgetIndex(0);
		NoticeTap->SetIsEnabled(false);
		InformationTap->SetIsEnabled(true);
		DirectMessageTap->SetIsEnabled(true);
		break;
	case ENoticeState::Information:
		NoticeWS->SetActiveWidgetIndex(1);
		NoticeTap->SetIsEnabled(true);
		InformationTap->SetIsEnabled(false);
		DirectMessageTap->SetIsEnabled(true);
		break;
	case ENoticeState::DM:
		NoticeWS->SetActiveWidgetIndex(2);
		NoticeTap->SetIsEnabled(true);
		InformationTap->SetIsEnabled(true);
		DirectMessageTap->SetIsEnabled(false);
		break;
	}
}


// Switch Buttons
void UNoticeUI::OnSwitchToNotice()
{
	SortNotices(ConfirmedVBox, UnConfirmedVBox);
	ChangeNoticeState(ENoticeState::Notice);
}

void UNoticeUI::OnSwitchToInformation()
{
	ChangeNoticeState(ENoticeState::Information);
}

void UNoticeUI::OnSwitchToDM()
{
	//SortNotices(DMConfirmedVBox, DMUnConfirmedVBox);
	ChangeNoticeState(ENoticeState::DM);
}


void UNoticeUI::OnToggleNoticeVisibility()
{
	if (IsAnimationPlaying(NoticeUI_SildeUpAnim))
		return;
	
	if (bIsNoticeVisible)
	{
		bIsNoticeVisible = false;
		PlayAnimation(NoticeUI_SildeUpAnim, 0, 1, EUMGSequencePlayMode::Reverse);
	}
	else
	{
		switch (CurNoticeState)
		{
		case ENoticeState::Notice:
			SortNotices(ConfirmedVBox, UnConfirmedVBox);
			break;
		case ENoticeState::Information:
			break;
		case ENoticeState::DM:
			//SortNotices(DMConfirmedVBox, DMUnConfirmedVBox);
			break;
		}
		bIsNoticeVisible = true;
		PlayAnimation(NoticeUI_SildeUpAnim);
		
		if (UnConfirmedVBox->GetChildrenCount() == 0)
		{
			Cast<ACuteAlienController>(GetOwningPlayer())->PlayerUI->NewNoticeBorder->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UNoticeUI::SortNotices(UVerticalBox* ConfirmedBox, UVerticalBox* UnConfirmedBox)
{
	TArray<UNoticeContentUI*> AllNoticeUIs;

	// 1️⃣ 모든 공지 아이템 수집
	for (UWidget* Child : UnConfirmedBox->GetAllChildren())
	{
		if (UNoticeContentUI* NoticeUI = Cast<UNoticeContentUI>(Child))
		{
			AllNoticeUIs.Add(NoticeUI);
		}
	}

	for (UWidget* Child : ConfirmedBox->GetAllChildren())
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
	UnConfirmedBox->ClearChildren();
	ConfirmedBox->ClearChildren();

	for (UNoticeContentUI* NoticeUI : AllNoticeUIs)
	{
		if (NoticeUI->IsConfirmed())
		{
			ConfirmedBox->AddChild(NoticeUI);
		}
		else
		{
			UnConfirmedBox->AddChild(NoticeUI);
		}
	}
}

void UNoticeUI::AddNotice(const FDispatchPayloadBase& Data)
{
	if (TSubclassOf<UNoticeContentUI> UNoticeContentUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<
		UNoticeContentUI>("WBP_NoticeContent"))
	{
		NoticeContentUI = CreateWidget<UNoticeContentUI>(this, UNoticeContentUIClass);
		NoticeContentUI->InitUI(Data);
		UnConfirmedVBox->InsertChildAt(0, NoticeContentUI);
	}
}

void UNoticeUI::AddDM(const FDispatchPayloadBase& Data)
{
	if (TSubclassOf<UNoticeContentUI> UNoticeContentUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<
		UNoticeContentUI>("WBP_NoticeContent"))
	{
		NoticeContentUI = CreateWidget<UNoticeContentUI>(this, UNoticeContentUIClass);
		NoticeContentUI->InitUI(Data);
		DMUnConfirmedVBox->InsertChildAt(0, NoticeContentUI);
	}
}
