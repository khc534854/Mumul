// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/PlayerUI.h"

#include "Components/Button.h"
#include "khc/Player/VoiceChatComponent.h"
#include "Yeomin/Player/CuteAlienController.h"
#include "Yeomin/Player/CuteAlienPlayer.h"

void UPlayerUI::NativeConstruct()
{
	Super::NativeConstruct();

	TentBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnTentClicked);
	MicrophoneBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnMicClicked);
	RecordBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnRecordClicked);

	PC = Cast<ACuteAlienController>(GetOwningPlayer());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("RadialUI Can't Find PlayerController"))
	}
}

UVoiceChatComponent* UPlayerUI::GetVoiceComponent() const
{
	if (!PC) return nullptr;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return nullptr;

	// GetComponentByClass는 느릴 수 있으니, 가능하다면 캐릭터에 캐싱된 변수를 쓰는 게 좋습니다.
	// 여기서는 안전하게 FindComponentByClass를 사용합니다.
	return Pawn->FindComponentByClass<UVoiceChatComponent>();
}

void UPlayerUI::OnTentClicked()
{
	PC->ShowPreviewTent();
}

void UPlayerUI::OnMicClicked()
{
	UVoiceChatComponent* VoiceComp = GetVoiceComponent();
	if (!VoiceComp) return;
	
	bIsSpeaking = !bIsSpeaking;
	
	bIsSpeaking ? VoiceComp->StartSpeaking() : VoiceComp->StopSpeaking();
}

void UPlayerUI::OnRecordClicked()
{
	UVoiceChatComponent* VoiceComp = GetVoiceComponent();
	if (!VoiceComp) return;
	
	bIsRecoding = !bIsRecoding;

	bIsRecoding ? VoiceComp->StartRecording() : VoiceComp->StopRecording();
}
