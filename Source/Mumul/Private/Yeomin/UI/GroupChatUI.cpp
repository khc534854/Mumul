// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/GroupChatUI.h"

#include "HttpNetworkSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "khc/Player/MumulPlayerState.h"
#include "khc/System/NetworkStructs.h"
#include "Yeomin/Player/CuteAlienController.h"
#include "Yeomin/UI/ChatBlockUI.h"
#include "Yeomin/UI/ChatMessageBlockUI.h"
#include "Yeomin/UI/CreateGroupChatUI.h"
#include "Yeomin/UI/GroupIconUI.h"
#include "Yeomin/UI/InvitationUI.h"

void UGroupChatUI::NativeConstruct()
{
	Super::NativeConstruct();

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

	// Register HTTP Response callback function
	HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>();
	if (HttpSystem)
	{
		HttpSystem->OnTeamChatListResponse.AddDynamic(this, &UGroupChatUI::OnServerTeamChatListResponse);
		HttpSystem->OnChatMessageResponse.AddDynamic(this, &UGroupChatUI::OnServerChatMessageResponse);
	}
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

		if (UChatBlockUI* ChatChunk = Cast<UChatBlockUI>(ChatSizeBox->GetChildAt(0)))
		{
			FString TeamID = ChatChunk->GetTeamID();
			TArray<int32> UserIDs;
			ChatChunk->GetTeamUsers().GetKeys(UserIDs);
			FString Content = Text.ToString();
			FString TimeStamp = FDateTime::Now().ToString(TEXT("%H:%M"));

			// Send Chat Message to DB
			HttpSystem->SendChatMessageRequest(TeamID, PS->PS_UserIndex, Content, TimeStamp);

			// Request Chat for Client RPC
			FString Player = PS->PS_RealName;
			PC->Server_RequestChat(TeamID, UserIDs, TimeStamp, Player, Content);

			// Init EditBox
			EditBox->SetText(FText());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GroupChatUI->ChatSizeBox has no Child (UChatBlockUI)"))
		}
	}
	// 
	else if (CommitMethod == ETextCommit::OnCleared)
	{
		// Focus EditBox
		EditBox->SetFocus();
	}
}

void UGroupChatUI::OnServerChatMessageResponse(bool bSuccess, FString Message)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s"), *Message);
	}
}

void UGroupChatUI::AddChat(const FString& TeamID, const FString& CurrentTime, const FString& Name,
                           const FString& Text) const
{
	if (UChatBlockUI* ChatChunk = Cast<UChatBlockUI>(ChatSizeBox->GetChildAt(0)))
	{
		// Does Group Name Match?
		if (ChatChunk->GetTeamID() != TeamID)
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

void UGroupChatUI::OnServerTeamChatListResponse(bool bSuccess, FString Message)
{
	if (bSuccess)
	{
		// 1. JSON 파싱 (Message에는 JSON 원본이 들어있음)
		TArray<FTeamChatListResponse> TeamChatList;

		if (FJsonObjectConverter::JsonArrayStringToUStruct(Message, &TeamChatList, 0, 0))
		{
			// JSON Parsing LOG
			for (const FTeamChatListResponse& TeamChat : TeamChatList)
			{
				UE_LOG(LogTemp, Warning, TEXT("===== 팀 정보 ====="));
				UE_LOG(LogTemp, Warning, TEXT("팀 ID: %s"), *TeamChat.teamChatId);
				UE_LOG(LogTemp, Warning, TEXT("팀 이름: %s"), *TeamChat.teamName);

				UE_LOG(LogTemp, Warning, TEXT("팀원 수: %d"), TeamChat.users.Num());
				for (const FUserDetail& User : TeamChat.users)
				{
					UE_LOG(LogTemp, Warning, TEXT("   - 유저ID: %d, 유저명: %s"),
					       User.userId,
					       *User.userName
					);
				}
			}
			GroupScrollBox->ClearChildren();
			for (const FTeamChatListResponse& TeamChat : TeamChatList)
			{
				// Create Group Icon
				UGroupIconUI* GroupIconUI = CreateWidget<UGroupIconUI>(GetWorld(), GroupIconUIClass);
				AddGroupIcon(GroupIconUI);
				GroupIconUI->InitParentUI(this);
				GroupIconUI->ChatBlockUI->SetTeamID(TeamChat.teamChatId);
				GroupIconUI->ChatBlockUI->SetTeamName(TeamChat.teamName);
				for (const FUserDetail& User : TeamChat.users)
				{
					GroupIconUI->ChatBlockUI->AddTeamUser(User.userId, *User.userName);
				}
				
				if (ACuteAlienController* PS = Cast<ACuteAlienController>(GetOwningPlayer()))
				{
					PS->Server_AddTeamChatList(TeamChat.teamChatId);
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("TeamChatList 파싱 실패"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("TeamChatList Response 실패 : %s"), *Message);
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

	AMumulPlayerState* PS = Cast<AMumulPlayerState>(GetOwningPlayerState());
	HttpSystem->SendTeamChatListRequest(PS->PS_UserIndex);
}
