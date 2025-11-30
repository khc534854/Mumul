// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/GroupChatUI.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
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
	DeleteBtn->OnPressed.AddDynamic(this, &UGroupChatUI::ShowDeleteUI);
}

void UGroupChatUI::ToggleVisibility(UWidget* Widget)
{
	const bool bIsVisible = (Widget->GetVisibility() == ESlateVisibility::Visible);
	Widget->SetVisibility(bIsVisible ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
}

void UGroupChatUI::AddChatBlock(UChatBlockUI* UI)
{
	ChatSizeBox->AddChild(UI);
}

void UGroupChatUI::RemoveChatBlock()
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
		// UGroupIconUI->GetPlayersInGroup()
		//TODO: ps->ServerRPC_SendChat(text.ToString());
		UChatBlockUI* ChatChunk = Cast<UChatBlockUI>(ChatSizeBox->GetChildAt(0));
		PC->Server_RequestChat(ChatChunk->GetPlayersInGroup(), Text.ToString(), PS->PS_RealName, FDateTime::Now().ToString(TEXT("%H:%M")));

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

void UGroupChatUI::AddChat(FString Text, FString Name, FString CurrentTime)
{
	if (UChatBlockUI* ChatChunck = Cast<UChatBlockUI>(ChatSizeBox->GetChildAt(0)))
	{
		// Scroll Current Location
		float ScrollOffset = ChatChunck->ChatScrollBox->GetScrollOffset();
		// Scroll End Location
		float EndOfScrollOffset = ChatChunck->ChatScrollBox->GetScrollOffsetOfEnd();

		// Add Chat Chunk to ScrollBox
		UChatMessageBlockUI* Chat = CreateWidget<UChatMessageBlockUI>(GetWorld(), ChatMessageBlockUIClass);
		ChatChunck->ChatScrollBox->AddChild(Chat);
		Chat->SetContent(Text, Name, CurrentTime);

		// If Scroll is at End
		if (ScrollOffset == EndOfScrollOffset)
		{
			// Scroll To End after 0.01s
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, [ChatChunck]()
			{
				// Scroll To End
				ChatChunck->ChatScrollBox->ScrollToEnd();
			}, 0.01f, false);
		}
	}
}

void UGroupChatUI::ToggleCreateGroupChatUI()
{
	ToggleVisibility(CreateGroupChatBox);
	if (InvitationBox->GetVisibility() == ESlateVisibility::Visible)
	{
		ToggleVisibility(InvitationBox);
	}
}

void UGroupChatUI::AddGroupIcon(UGroupIconUI* UI)
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

void UGroupChatUI::ShowDeleteUI()
{
}
