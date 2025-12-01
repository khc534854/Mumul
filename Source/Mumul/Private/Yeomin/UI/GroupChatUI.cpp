// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/GroupChatUI.h"

#include "Base/MumulGameState.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "khc/Player/MumulPlayerState.h"
#include "Yeomin/Player/CuteAlienController.h"
#include "Yeomin/UI/ChatBlockUI.h"
#include "Yeomin/UI/ChatMessageBlockUI.h"
#include "Yeomin/UI/CreateGroupChatUI.h"
#include "Yeomin/UI/GroupIconUI.h"
#include "Yeomin/UI/InvitationUI.h"

void UGroupChatUI::NativeConstruct()
{
	Super::NativeConstruct();

	// Register Text Commit callback function
	EditBox->OnTextCommitted.AddDynamic(this, &UGroupChatUI::OnTextBoxCommitted);

	AddGroupBtn->OnPressed.AddDynamic(this, &UGroupChatUI::ToggleCreateGroupChatUI);

	CreateGroupChatUI = CreateWidget<UCreateGroupChatUI>(this, CreateGroupChatUIClass);
	CreateGroupChatUI->InitParentUI(this);
	CreateGroupChatBox->AddChild(CreateGroupChatUI);
	CreateGroupChatBox->SetVisibility(ESlateVisibility::Hidden);

	InvitationUI = CreateWidget<UInvitationUI>(this, InvitationUIClass);
	InvitationBox->AddChild(InvitationUI);
	InvitationBox->SetVisibility(ESlateVisibility::Hidden);

	InviteBtn->OnPressed.AddDynamic(this, &UGroupChatUI::ToggleInvitationUI);
	ToggleVisibilityBtn->OnPressed.AddDynamic(this, &UGroupChatUI::OnToggleVisibilityBtn);
}

void UGroupChatUI::ToggleVisibility(UWidget* Widget)
{
	const bool bIsVisible = (Widget->GetVisibility() == ESlateVisibility::Visible);
	Widget->SetVisibility(bIsVisible ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
}

void UGroupChatUI::AddChatBlock(UChatBlockUI* UI) const
{
	ChatSizeBox->AddChild(UI);
}

void UGroupChatUI::RemoveChatBlock() const
{
	ChatSizeBox->ClearChildren();
}

void UGroupChatUI::OnTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// If On Enter
	if (CommitMethod == ETextCommit::OnEnter)
	{
		ACuteAlienController* PC = Cast<ACuteAlienController>(GetWorld()->GetFirstPlayerController());
		AMumulPlayerState* PS = PC->GetPlayerState<AMumulPlayerState>();

		UChatBlockUI* ChatChunk = Cast<UChatBlockUI>(ChatSizeBox->GetChildAt(0));
		FString TimeStamp = FDateTime::Now().ToString(TEXT("%H:%M"));
		FString Player = PS->PS_RealName;
		FString Content = Text.ToString();
		FString GroupName = ChatChunk->GetGroupName();
		PC->Server_RequestChatHistory(GroupName, Player, TimeStamp, Content);
		PC->Server_RequestChat(GroupName, ChatChunk->GetPlayersInGroup(), TimeStamp, Player, Content);

		// Init EditBox
		EditBox->SetText(FText());
	}
	// 
	else if (CommitMethod == ETextCommit::OnCleared)
	{
		// Focus EditBox
		EditBox->SetFocus();
	}
}

void UGroupChatUI::AddChat(const FString& Group, const FString& CurrentTime, const FString& Name,
                           const FString& Text) const
{
	if (UChatBlockUI* ChatChunk = Cast<UChatBlockUI>(ChatSizeBox->GetChildAt(0)))
	{
		// Does Group Name Match?
		if (ChatChunk->GetGroupName() != Group)
			return;

		// Scroll Current Location
		const float ScrollOffset = ChatChunk->ChatScrollBox->GetScrollOffset();
		// Scroll End Location
		const float EndOfScrollOffset = ChatChunk->ChatScrollBox->GetScrollOffsetOfEnd();

		// Add Chat Chunk to ScrollBox
		UChatMessageBlockUI* Chat = CreateWidget<UChatMessageBlockUI>(GetWorld(), ChatMessageBlockUIClass);
		ChatChunk->ChatScrollBox->AddChild(Chat);
		Chat->SetContent(CurrentTime, Name, Text);

		// If Scroll is at End
		if (ScrollOffset == EndOfScrollOffset)
		{
			// Scroll To End after 0.01s
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, [ChatChunk]()
			{
				// Scroll To End
				ChatChunk->ChatScrollBox->ScrollToEnd();
			}, 0.01f, false);
		}
	}
}

void UGroupChatUI::SetGroupNameTitle(const FString& GroupName)
{
	GroupNameTitle->SetText(FText::FromString(GroupName));
}

void UGroupChatUI::ToggleCreateGroupChatUI()
{
	ToggleVisibility(CreateGroupChatBox);
	if (InvitationBox->GetVisibility() == ESlateVisibility::Visible)
	{
		ToggleVisibility(InvitationBox);
	}
}

void UGroupChatUI::AddGroupIcon(UGroupIconUI* UI) const
{
	GroupScrollBox->AddChild(UI);
}

void UGroupChatUI::ToggleInvitationUI()
{
	ToggleVisibility(InvitationBox);
	if (CreateGroupChatBox->GetVisibility() == ESlateVisibility::Visible)
	{
		ToggleVisibility(CreateGroupChatBox);
	}
}

void UGroupChatUI::OnToggleVisibilityBtn()
{
	CreateGroupChatUI->RefreshJoinedPlayerList();
	AMumulGameState* GS = Cast<AMumulGameState>(GetWorld()->GetGameState());
	
	UE_LOG(LogTemp, Warning, TEXT("===== GroupChatList Dump ====="));

	for (const FGroupChatData& GroupData : GS->GetGroupChatHistory())
	{
		UE_LOG(LogTemp, Warning, TEXT("Group: %s (Count = %d)"),
			*GroupData.GroupName, GroupData.ChatBlocks.Num());

		for (int32 i = 0; i < GroupData.ChatBlocks.Num(); i++)
		{
			const FChatBlock& Block = GroupData.ChatBlocks[i];

			UE_LOG(LogTemp, Warning,
				TEXT("   [%d] Time: %s | Player: %s | Content: %s"),
				i,
				*Block.TimeStamp,
				*Block.PlayerName,
				*Block.Content
			);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("===== End Dump ====="));
}
