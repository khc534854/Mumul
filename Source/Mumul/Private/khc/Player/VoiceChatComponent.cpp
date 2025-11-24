#include "khc/Player/VoiceChatComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

UVoiceChatComponent::UVoiceChatComponent()
{
	PrimaryComponentTick.bCanEverTick = false; 
}


void UVoiceChatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UVoiceChatComponent::StartSpeaking()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	if (!OwnerPawn->IsLocallyControlled()) return;

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		UE_LOG(LogTemp, Log, TEXT("[VoiceComponent] Start Talking"));
		PC->StartTalking();
	}
}

void UVoiceChatComponent::StopSpeaking()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	if (!OwnerPawn->IsLocallyControlled()) return;

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		UE_LOG(LogTemp, Log, TEXT("[VoiceComponent] Stop Talking"));
		PC->StopTalking();
	}
}
