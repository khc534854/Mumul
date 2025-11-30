// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/CreateGroupChatUI.h"

#include "Base/MumulGameState.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
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
		GS->OnPlayerArrayUpdated.AddDynamic(this, &UCreateGroupChatUI::RefreshJoinedPlayerList);
		RefreshJoinedPlayerList();
	}

	CreateGroupBtn->OnPressed.AddDynamic(this, &UCreateGroupChatUI::CreateGroupChat);
	SearchBox->OnTextChanged.AddDynamic(this, &UCreateGroupChatUI::OnSearchTextChanged);
}

void UCreateGroupChatUI::RefreshJoinedPlayerList()
{
	if (GS)
	{
		PlayerScrollBox->ClearChildren();
		for (APlayerState* PS : GS->PlayerArray)
		{
			UGroupProfileUI* ProfileUI = CreateWidget<UGroupProfileUI>(GetWorld(), GroupProfileUIClass);
			PlayerScrollBox->AddChild(ProfileUI);
			// 플레이어 이름 갱신
			ProfileUI->SetPlayerName(PS->GetPlayerName());
		}
	}
}

void UCreateGroupChatUI::OnSearchTextChanged(const FText& Text)
{
	GetWorld()->GetTimerManager().ClearTimer(SearchDelayTimer);

	GetWorld()->GetTimerManager().SetTimer
	(
		SearchDelayTimer,
		[this, Text]()
		{
			RefreshFilteredPlayerList(Text);
		},
		0.1f,
		false
	);
}

void UCreateGroupChatUI::RefreshFilteredPlayerList(const FText& Text)
{
	TArray<UWidget*> Children = PlayerScrollBox->GetAllChildren();

	for (UWidget* Child : Children)
	{
		UGroupProfileUI* ProfileUI = Cast<UGroupProfileUI>(Child);
		
		const FString Name = ProfileUI->GetPlayerName();
		
		if (Text.IsEmpty())
		{
			ProfileUI->SetVisibility(ESlateVisibility::Visible);
			continue;
		}
		
		const bool bIsContaining = Name.Contains(Text.ToString());
		ProfileUI->SetVisibility(bIsContaining ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
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
