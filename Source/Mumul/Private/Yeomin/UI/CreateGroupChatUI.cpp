// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/CreateGroupChatUI.h"

#include "HttpNetworkSubsystem.h"
#include "Base/MumulGameState.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "GameFramework/PlayerState.h"
#include "khc/Player/MumulPlayerState.h"
#include "Yeomin/Player/CuteAlienController.h"
#include "Yeomin/UI/GroupChatUI.h"
#include "Yeomin/UI/GroupProfileUI.h"

void UCreateGroupChatUI::NativeConstruct()
{
	Super::NativeConstruct();

	GS = GetWorld()->GetGameState<AMumulGameState>();
	if (GS)
	{
		GS->OnPlayerArrayUpdated.AddDynamic(this, &UCreateGroupChatUI::RefreshJoinedPlayerList);
	}
	CreateGroupBtn->OnPressed.AddDynamic(this, &UCreateGroupChatUI::CreateGroupChat);
	SearchBox->OnTextChanged.AddDynamic(this, &UCreateGroupChatUI::OnSearchTextChanged);
	RefreshJoinedPlayerList();

	HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>();
}

void UCreateGroupChatUI::RefreshJoinedPlayerList()
{
	if (GS)
	{
		PlayerScrollBox->ClearChildren();
		for (APlayerState* PS : GS->PlayerArray)
		{
			AMumulPlayerState* MPS = Cast<AMumulPlayerState>(PS);

			UGroupProfileUI* ProfileUI = CreateWidget<UGroupProfileUI>(GetWorld(), GroupProfileUIClass);
			PlayerScrollBox->AddChild(ProfileUI);
			ProfileUI->SetPlayerName(MPS->PS_RealName);
			ProfileUI->SetUserIndex(MPS->PS_UserIndex);
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
	// Check Players
	TArray<UWidget*> Children = PlayerScrollBox->GetAllChildren();
	TArray<int32> CheckedUserIDs;

	for (UWidget* Child : Children)
	{
		UGroupProfileUI* ProfileUI = Cast<UGroupProfileUI>(Child);
		if (ProfileUI->GetCheckBoxState())
		{
			CheckedUserIDs.Add(ProfileUI->GetUserIndex());
		}
	}

	if (CheckedUserIDs.Num() == 0)
		return;

	FString GroupName = GroupNameText->GetText().ToString();
	GroupName = MakeUniqueGroupName(GroupName);
	
	// Send CreateTeamChat Request
	HttpSystem->SendCreateTeamChatRequest(GroupName, CheckedUserIDs);

	ParentUI->ToggleCreateGroupChatUI();
}

FString UCreateGroupChatUI::MakeUniqueGroupName(const FString& BaseName) const
{
	const FString NewBaseName = BaseName.IsEmpty() ? TEXT("그룹") : BaseName;

	int32 MaxIndex = 0;
	bool bIsNameExisting = false;

	// Search TeamChatList
	for (const FString& TeamName : GS->GetTeamChatList())
	{
		const FString& OtherName = TeamName;

		// if Same Name?
		if (OtherName == NewBaseName)
		{
			bIsNameExisting = true;
			continue;
		}

		// if Name starts with BaseName
		if (OtherName.StartsWith(NewBaseName))
		{
			// Check Suffix (BaseName + Number)
			const FString Suffix = OtherName.RightChop(NewBaseName.Len());

			if (Suffix.IsNumeric())
			{
				const int32 Num = FCString::Atoi(*Suffix);
				MaxIndex = FMath::Max(MaxIndex, Num);
			}
		}
	}

	// if No Name Existing
	if (!bIsNameExisting)
	{
		return NewBaseName;
	}

	// BaseName + (MaxIndex + 1)
	return FString::Printf(TEXT("%s%d"), *NewBaseName, MaxIndex + 1);
}
