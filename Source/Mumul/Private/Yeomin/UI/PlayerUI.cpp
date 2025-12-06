// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/PlayerUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "khc/Player/VoiceChatComponent.h"
#include "Yeomin/Player/CuteAlienController.h"
#include "Yeomin/UI/GroupChatUI.h"
#include "Yeomin/UI/BaseUI/BaseText.h"

void UPlayerUI::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateCurrentTime();
	FDateTime Now = FDateTime::Now();
	int32 RemainingSeconds = 60 - Now.GetSecond();
	GetWorld()->GetTimerManager().SetTimer(
		FirstMinuteTimer,
		this,
		&UPlayerUI::StartMinuteTimer,
		RemainingSeconds,
		false
	);

	TentBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnTentClicked);
	MicrophoneBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnMicClicked);

	GetWorld()->GetTimerManager().SetTimer(
		GroupChatCheckTimer,
		this,
		&UPlayerUI::CheckGroupChatUI,
		0.7f,
		true
	);

	PC = Cast<ACuteAlienController>(GetOwningPlayer());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("RadialUI Can't Find PlayerController"))
	}

	if (UVoiceChatComponent* VoiceComp = GetVoiceComponent())
	{
		UpdateMicButtonState(VoiceComp->IsSpeaking());
		UpdateRecordButtonState(VoiceComp->IsRecording());

		VoiceComp->OnSpeakingStateChanged.AddDynamic(this, &UPlayerUI::UpdateMicButtonState);
		VoiceComp->OnRecordingStateChanged.AddDynamic(this, &UPlayerUI::UpdateRecordButtonState);
	}

	LogOutBtn->SetVisibility(ESlateVisibility::Hidden);

	// 델리게이트 바인딩
	ProfileBtn->OnHovered.AddDynamic(this, &UPlayerUI::OnProfileBtnHovered);
	ProfileBtn->OnUnhovered.AddDynamic(this, &UPlayerUI::OnProfileBtnUnhovered);

	LogOutBtn->OnHovered.AddDynamic(this, &UPlayerUI::OnLogOutBtnHovered);
	LogOutBtn->OnUnhovered.AddDynamic(this, &UPlayerUI::OnLogOutBtnUnhovered);
}

void UPlayerUI::StartMinuteTimer()
{
	UpdateCurrentTime();
	
	GetWorld()->GetTimerManager().SetTimer(
		TimeUpdater,
		this,
		&UPlayerUI::UpdateCurrentTime,
		60.f,
		true
	);
}

void UPlayerUI::UpdateCurrentTime()
{
	FDateTime Now = FDateTime::Now();
	FString TimeString = Now.ToString(TEXT("%H:%M"));
	if (CurrentTime)
	{
		CurrentTime->BaseText->SetText(FText::FromString(TimeString));
	}
}

UVoiceChatComponent* UPlayerUI::GetVoiceComponent() const
{
	if (!PC) return nullptr;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return nullptr;

	return Pawn->FindComponentByClass<UVoiceChatComponent>();
}

void UPlayerUI::CheckGroupChatUI()
{
	if (GroupChatUI && GroupChatUI->RecordBtn)
	{
		GroupChatUI->RecordBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnRecordClicked);
		
		GetWorld()->GetTimerManager().ClearTimer(GroupChatCheckTimer);
	}
}

void UPlayerUI::InitGroupChatUI(UGroupChatUI* UI)
{
	GroupChatUI = UI;
}

void UPlayerUI::UpdateMicButtonState(bool bActive)
{
	if (bActive)
	{
		PlayAnimation(MicOn, 0, 0);
	}
	else
	{
		StopAnimation(MicOn);
	}
	ChangeMicStateImage();
}

void UPlayerUI::UpdateRecordButtonState(bool bActive)
{
}

void UPlayerUI::OnTentClicked()
{
	PC->ShowPreviewTent();
}

void UPlayerUI::OnMicClicked()
{
	if (UVoiceChatComponent* VoiceComp = GetVoiceComponent())
	{
		VoiceComp->ToggleSpeaking();
	}
}

void UPlayerUI::OnRecordClicked()
{
	if (UVoiceChatComponent* VoiceComp = GetVoiceComponent())
	{
		if (PC)
		{
			// 녹음 중이면 -> 종료 팝업 띄우기
			if (VoiceComp->IsRecording())
			{
				PC->OpenEndMeetingPopup();
			}
			// 녹음 중 아니면 -> 시작 설정 UI 띄우기
			else
			{
				PC->OpenMeetingSetupUI();
			}
		}
	}
}

void UPlayerUI::OnProfileBtnHovered()
{
	bIsMainHovered = true;
	GetWorld()->GetTimerManager().ClearTimer(HideLogOutTimer);
	bIsTryingToHide = false;
	LogOutBtn->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerUI::OnProfileBtnUnhovered()
{
	bIsMainHovered = false;
	TryHideLogOutBtn();
}

void UPlayerUI::OnLogOutBtnHovered()
{
	bIsSubHovered = true;
	GetWorld()->GetTimerManager().ClearTimer(HideLogOutTimer);
	bIsTryingToHide = false;
}

void UPlayerUI::OnLogOutBtnUnhovered()
{
	bIsSubHovered = false;
	TryHideLogOutBtn();
}

void UPlayerUI::TryHideLogOutBtn()
{
	if (bIsTryingToHide)
		return;

	bIsTryingToHide = true;
	
	GetWorld()->GetTimerManager().SetTimer(
		HideLogOutTimer,
		this,
		&UPlayerUI::HideLogOutBtn,
		0.05f,
		false
	);
}

void UPlayerUI::HideLogOutBtn()
{
	bIsTryingToHide = false;
	
	if (bIsMainHovered || bIsSubHovered)
		return;

	LogOutBtn->SetVisibility(ESlateVisibility::Hidden);
}
