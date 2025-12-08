// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/InvitationUI.h"

#include "Base/MumulGameState.h"
#include "Components/ScrollBox.h"
#include "GameFramework/PlayerState.h"
#include "Yeomin/Player/CuteAlienController.h"
#include "Yeomin/UI/GroupProfileUI.h"

void UInvitationUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	ACuteAlienController* PS = Cast<ACuteAlienController>(GetOwningPlayer());
	if (PS)
	{
		PS->OnPlayerArrayUpdated.AddDynamic(this, &UInvitationUI::RefreshJoinedPlayerList);
	}
	
	GS = GetWorld()->GetGameState<AMumulGameState>();
}

void UInvitationUI::RefreshJoinedPlayerList()
{
	if (GS)
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			UGroupProfileUI* ProfileUI = CreateWidget<UGroupProfileUI>(GetWorld(), GroupProfileUIClass);
			PlayerScrollBox->AddChild(ProfileUI);
			ProfileUI->SetPlayerName(PS->GetPlayerName());
		}
	}
}
