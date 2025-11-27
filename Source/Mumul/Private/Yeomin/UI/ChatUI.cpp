// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/ChatUI.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "khc/Player/MumulPlayerState.h"
#include "Yeomin/UI/ChatMessageBlockUI.h"


void UChatUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	static ConstructorHelpers::FClassFinder<UChatMessageBlockUI> ChatChunkUIFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/WBP_ChatMessageUI.WBP_ChatMessageUI_C"));
	if (ChatChunkUIFinder.Succeeded())
	{
		ChatMessageBlockUIClass = ChatChunkUIFinder.Class;
	}

	// Register Text Commit callback function
	EditBox->OnTextCommitted.AddDynamic(this, &UChatUI::OnTextBoxCommitted);
}

void UChatUI::OnTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// If On Enter
	if (CommitMethod == ETextCommit::OnEnter)
	{
		// 서버에게 채팅 내용 전달
		// 내 PlayerState 가져오자.
		APlayerController* pc = GetWorld()->GetFirstPlayerController();
		AMumulPlayerState* ps = pc->GetPlayerState<AMumulPlayerState>();
		//TODO: ps->ServerRPC_SendChat(text.ToString());

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

void UChatUI::AddChat(FString Text)
{
	// Scroll Current Location
	float ScrollOffset = ScrollBox->GetScrollOffset();
	// Scroll End Location
	float EndOfScrollOffset = ScrollBox->GetScrollOffsetOfEnd();
	
	// Add Chat Chunk to ScrollBox
	UChatMessageBlockUI* Chat = CreateWidget<UChatMessageBlockUI>(GetWorld(), ChatMessageBlockUIClass);
	ScrollBox->AddChild(Chat);
	Chat->SetContent(Text);
	
	// If Scroll is at End
	if (ScrollOffset == EndOfScrollOffset)
	{
		// Scroll To End after 0.01s
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
		{
			// Scroll To End
			ScrollBox->ScrollToEnd();
		}, 0.01f, false);
	}
}