// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/CreateGroupChatUI.h"

#include "Base/MumulGameState.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "GameFramework/PlayerState.h"
#include "Yeomin/UI/GroupChatUI.h"
#include "Yeomin/UI/GroupIconUI.h"
#include "Yeomin/UI/GroupProfileUI.h"

void UCreateGroupChatUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	GS = GetWorld()->GetGameState<AMumulGameState>();
	if (GS)
	{
		GS->OnPlayerArrayUpdated.AddDynamic(this, &UCreateGroupChatUI::RefreshPlayerList);
		RefreshPlayerList();
	}
	
	CreateGroupBtn->OnPressed.AddDynamic(this, &UCreateGroupChatUI::CreateGroupChat);
}

void UCreateGroupChatUI::RefreshPlayerList()
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

void UCreateGroupChatUI::InitParentUI(UGroupChatUI* Parent)
{
	ParentUI = Parent;
}

void UCreateGroupChatUI::CreateGroupChat()
{
	UGroupIconUI* GroupIconUI = CreateWidget<UGroupIconUI>(GetWorld(), GroupIconUIClass);
	GroupIconUI->InitParentUI(ParentUI);
	ParentUI->AddGroupIcon(GroupIconUI);
}
