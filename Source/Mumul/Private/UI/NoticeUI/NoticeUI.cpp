// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NoticeUI/NoticeUI.h"

#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/WidgetSwitcher.h"
#include "Data/AudioManager.h"
#include "Data/ObjectAndClassFinder.h"
#include "Algo/StableSort.h"
#include "UI/NoticeUI/DispatchPopUI.h"
#include "UI/NoticeUI/NoticeContentUI.h"

void UNoticeUI::NativeConstruct()
{
	Super::NativeConstruct();

	NoticeTap->OnClicked.AddDynamic(this, &UNoticeUI::OnSwitchToNotice);
	InformationTap->OnClicked.AddDynamic(this, &UNoticeUI::OnSwitchToInformation);
	DirectMessageTap->OnClicked.AddDynamic(this, &UNoticeUI::OnSwitchToDM);

	ChangeNoticeState(ENoticeState::Notice);

	AudioManager = GetGameInstance()->GetSubsystem<UAudioManager>();
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

	FLinearColor Color;
	
	switch (NewState)
	{
	case ENoticeState::Notice:
		NoticeWS->SetActiveWidgetIndex(0);
		NoticeTap->SetVisibility(ESlateVisibility::HitTestInvisible);
		DirectMessageTap->SetVisibility(ESlateVisibility::Visible);

		Color = FLinearColor::FromSRGBColor(FColor(70, 75, 95, 255));
		NoticeTap->SetColorAndOpacity(Color);
		DirectMessageTap->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f));
		break;
	case ENoticeState::DM:
		NoticeWS->SetActiveWidgetIndex(2);
		NoticeTap->SetVisibility(ESlateVisibility::Visible);
		DirectMessageTap->SetVisibility(ESlateVisibility::HitTestInvisible);
		
		Color = FLinearColor::FromSRGBColor(FColor(70, 75, 95, 255));
		DirectMessageTap->SetColorAndOpacity(Color);
		NoticeTap->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 1.f));
		break;
	default:
		return;
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
	SortDMs();
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
			SortDMs();
			break;
		}
		bIsNoticeVisible = true;
		AudioManager->PlayPopUpSound();
		PlayAnimation(NoticeUI_SildeUpAnim);
	}
}

void UNoticeUI::SortNotices(UVerticalBox* ConfirmedBox, UVerticalBox* UnConfirmedBox)
{
	TArray<UNoticeContentUI*> AllNoticeUIs;

	// 1️. 모든 공지 아이템 수집
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

	// 2. 상태 + 시간 기준 정렬
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

	// 3. 정렬 결과를 UI에 반영
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

void UNoticeUI::SortDMs()
{
	if (!DMVBox) return;

	TArray<UNoticeContentUI*> DMs;
	DMs.Reserve(DMVBox->GetChildrenCount());

	for (UWidget* Child : DMVBox->GetAllChildren())
	{
		if (UNoticeContentUI* DMUI = Cast<UNoticeContentUI>(Child))
		{
			DMs.Add(DMUI);
		}
	}

	DMs.Sort([](const UNoticeContentUI& A, const UNoticeContentUI& B)
	{
		const FDateTime TimeA = A.GetCreatedAt();
		const FDateTime TimeB = B.GetCreatedAt();

		// 1. 최신 시간 우선
		if (TimeA > TimeB) return true;
		if (TimeA < TimeB) return false;

		// 2. 시간 완전히 같을 때 → MessageId 큰 게 최신
		return A.GetMessageId() > B.GetMessageId();
	});

	DMVBox->ClearChildren();

	for (UNoticeContentUI* DMUI : DMs)
	{
		DMVBox->AddChild(DMUI);
	}
}

void UNoticeUI::AddNotice(const FDispatchPayloadBase& Data)
{
	if (TSubclassOf<UNoticeContentUI> NoticeContentUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<
		UNoticeContentUI>("WBP_NoticeContent"))
	{
		NoticeContentUI = CreateWidget<UNoticeContentUI>(GetOwningPlayer(), NoticeContentUIClass);
		NoticeContentUI->InitUI(Data);
		NoticeContentUI->SetTimeStampText(Data.CreatedAt);

		UnConfirmedVBox->AddChild(NoticeContentUI);

		SortNotices(ConfirmedVBox, UnConfirmedVBox);
	}
}

void UNoticeUI::AddDM(const FDispatchPayloadBase& Data)
{
	if (TSubclassOf<UNoticeContentUI> NoticeContentUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<
		UNoticeContentUI>("WBP_NoticeContent"))
	{
		NoticeContentUI = CreateWidget<UNoticeContentUI>(GetOwningPlayer(), NoticeContentUIClass);
		NoticeContentUI->InitUI(Data);
		NoticeContentUI->SetTimeStampText(Data.CreatedAt);

		DMVBox->AddChild(NoticeContentUI);

		SortDMs();
	}
}

void UNoticeUI::DisplayDispatchPopUp(const FDispatchPayloadBase& DispatchPayload)
{
	DispatchBox->ClearChildren();
	PlayAnimation(DispatchBox_SlideAnim);

	if (TSubclassOf<UDispatchPopUI> DispatchPopUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<
		UDispatchPopUI>("WBP_DispatchPop"))
	{
		DispatchPopUI = CreateWidget<UDispatchPopUI>(GetOwningPlayer(), DispatchPopUIClass);
		FString Content;
		if (DispatchPayload.Title.IsEmpty())
		{
			Content = FString::Printf(TEXT("[개인메시지] \n%s"), *DispatchPayload.Text);
		}
		else
		{
			Content = FString::Printf(TEXT("[%s] \n%s"), *DispatchPayload.Title, *DispatchPayload.Text);
		}
		DispatchPopUI->SetDispatchText(Content);
		DispatchBox->AddChild(DispatchPopUI);
	}
}

void UNoticeUI::HideDispatchPopUp()
{
	PlayAnimation(DispatchBox_SlideAnim, 0, 1, EUMGSequencePlayMode::Reverse);
}

bool UNoticeUI::IsAllConfirmed()
{
	// 미확인 VBox
	for (UWidget* Child : UnConfirmedVBox->GetAllChildren())
	{
		if (UNoticeContentUI* NoticeUI = Cast<UNoticeContentUI>(Child))
		{
			if (!NoticeUI->IsConfirmed())
			{
				return false;
			}
		}
	}

	// 확인 VBox
	for (UWidget* Child : ConfirmedVBox->GetAllChildren())
	{
		if (UNoticeContentUI* NoticeUI = Cast<UNoticeContentUI>(Child))
		{
			if (!NoticeUI->IsConfirmed())
			{
				return false;
			}
		}
	}

	return true;
}
