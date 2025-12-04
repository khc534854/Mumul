// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/GroupChatUI.h"

#include "HttpNetworkSubsystem.h"
#include "WebSocketSubsystem.h"
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
#include "MumulGameInstance.h" // 필수

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

	if (WebSocketSystem)
	{
		WebSocketSystem->OnAIChatStarted.AddDynamic(this, &UGroupChatUI::OnAIChatStarted);
		WebSocketSystem->OnAIChatAnswer.AddDynamic(this, &UGroupChatUI::OnAIChatAnswer);
	}

	// [신규] 챗봇 방 생성 및 상단 배치
	InitChatbotRoom();
}

void UGroupChatUI::ToggleVisibility(UWidget* Widget)
{
	const bool bIsVisible = (Widget->GetVisibility() == ESlateVisibility::Visible);
	Widget->SetVisibility(bIsVisible ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
}

void UGroupChatUI::SelectGroupChat(class UGroupIconUI* SelectedIcon)
{
	if (!SelectedIcon) return;
    if (CurrentSelectedGroup == SelectedIcon) return; 

    UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
    FString UserID = GI ? FString::FromInt(GI->PlayerUniqueID) : TEXT("Unknown");

    // 1. 이전 방 정리 (챗봇 방에서 나가는 경우)
    if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
    {
        if (WebSocketSystem && WebSocketSystem->IsConnected())
        {
            FWSRequest_EndChat EndReq;
            EndReq.sessionId = UserID;
            WebSocketSystem->SendStructMessage(EndReq);
            WebSocketSystem->Close();
        }
    }

    // 2. UI 교체
    RemoveChatBlock();
    if (SelectedIcon->ChatBlockUI)
    {
        AddChatBlock(SelectedIcon->ChatBlockUI);
        SetGroupNameTitle(SelectedIcon->ChatBlockUI->GetTeamName());
    }
    
    CurrentSelectedGroup = SelectedIcon;

    // 3. 새 방 진입 처리
    if (SelectedIcon->bIsChatbotRoom)
    {
        // [챗봇 방] 웹소켓 연결 시도
        if (SelectedIcon->ChatBlockUI && SelectedIcon->ChatBlockUI->ChatScrollBox->GetChildrenCount() == 0)
        {
            FTimerHandle WelcomeHandle;
            GetWorld()->GetTimerManager().SetTimer(WelcomeHandle, [this]()
            {
                AddBotChat(TEXT("안녕하세요! 무엇을 도와드릴까요?"));
            }, 0.1f, false);
        }

        if (WebSocketSystem)
        {
            FString WSUrl = TEXT("ws://127.0.0.1:8000/learning_chatbot/"); 
            WebSocketSystem->Connect(WSUrl);

            FTimerHandle ConnectTimerHandle;
            GetWorld()->GetTimerManager().SetTimer(ConnectTimerHandle, [this, UserID]()
            {
                if (!this || !WebSocketSystem) return;

                if (WebSocketSystem->IsConnected())
                {
                    FWSRequest_StartChat StartReq;
                    StartReq.sessionId = UserID;
                    StartReq.userId = UserID;
                    WebSocketSystem->SendStructMessage(StartReq);
                }
                else
                {
                     // 연결 대기 중...
                }
            }, 0.5f, false); 
        }
    }
    else
    {
        // [일반 방] 로직 복원
        
        // 1) HTTP로 지난 대화 내역 불러오기
        if (HttpSystem)
        {
            HttpSystem->SendTeamChatMessageRequest(SelectedIcon->ChatBlockUI->GetTeamID());
        }
        
        // 2) 보이스 채널 변경 (주석 해제 및 복원)
        if (AMumulPlayerState* PS = Cast<AMumulPlayerState>(GetOwningPlayerState()))
        {
             // TeamID(String)를 int32로 변환
             FString TargetChannelID = SelectedIcon->ChatBlockUI->GetTeamID();

             if (PS->bIsNearByCampFire)
             {
                PS->Server_SetVoiceChannelID(TargetChannelID);
             }
             else
             {
                PS->WaitingChannelID = TargetChannelID;
             }
             
             UE_LOG(LogTemp, Log, TEXT("[UI] Switched Voice Channel to: %s"), *TargetChannelID);
        }
    }
}

void UGroupChatUI::OnAIChatStarted(FString Message)
{
	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
	{
		AddBotChat(Message); // [수정] AddChat -> AddBotChat
	}
}

void UGroupChatUI::OnAIChatAnswer(FString Answer)
{
	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
	{
		AddBotChat(Answer); // [수정] AddChat -> AddBotChat
	}
}

void UGroupChatUI::InitChatbotRoom()
{
	if (!GroupIconUIClass || !ChatBlockUIClass) return;

	// 1. 챗봇용 아이콘 생성
	UGroupIconUI* ChatbotIcon = CreateWidget<UGroupIconUI>(GetWorld(), GroupIconUIClass);
	if (ChatbotIcon)
	{
		ChatbotIcon->InitParentUI(this);
		ChatbotIcon->bIsChatbotRoom = true; // 챗봇 방임을 표시

		// 2. 챗봇용 채팅 블록(내용창) 생성
		UChatBlockUI* ChatbotBlock = CreateWidget<UChatBlockUI>(GetWorld(), ChatBlockUIClass);
		ChatbotBlock->SetTeamID(TEXT("Chatbot_Room"));
		ChatbotBlock->SetTeamName(TEXT("무엇이든 물어보세요"));
        
		ChatbotIcon->ChatBlockUI = ChatbotBlock; // 아이콘에 연결

		// [수정] 메시지는 여기서 넣지 않음! (비워둠)

		// 3. [수정] 스크롤박스에 추가 (InsertAt 대신 AddChild 사용)
		GroupScrollBox->AddChild(ChatbotIcon);
	}
}

void UGroupChatUI::AddBotChat(const FString& Message)
{
	UChatBlockUI* ChatChunk = Cast<UChatBlockUI>(ChatSizeBox->GetChildAt(0));
	if (!ChatChunk) return;

	// 2. 챗봇용 말풍선 생성
	if (BotChatMessageBlockUIClass)
	{
		UChatMessageBlockUI* BotChat = CreateWidget<UChatMessageBlockUI>(GetWorld(), BotChatMessageBlockUIClass);
		if (BotChat)
		{
			ChatChunk->ChatScrollBox->AddChild(BotChat);
            
			FString TimeStamp = FDateTime::Now().ToString(TEXT("%H:%M"));
			BotChat->SetContent(TimeStamp, TEXT("무물이"), Message);

			// 스크롤 내리기
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, [ChatChunk]()
			{
				ChatChunk->ChatScrollBox->ScrollToEnd();
			}, 0.01f, false);
		}
	}
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
	if (CommitMethod != ETextCommit::OnEnter) 
	{
		if (CommitMethod == ETextCommit::OnCleared) EditBox->SetFocus();
		return;
	}

	if (Text.IsEmpty()) return;
	if (!CurrentSelectedGroup) return;

	// [수정] 변수들을 여기서 미리 선언
	FString Content = Text.ToString();
	FString TimeStamp = FDateTime::Now().ToString(TEXT("%H:%M"));
    
	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	FString MyName = GI ? GI->PlayerName : TEXT("Me");
	int32 MyID = GI ? GI->PlayerUniqueID : 0;

	// [내 메시지 UI에 즉시 추가]
	if (CurrentSelectedGroup->ChatBlockUI) // Null 체크 추가
	{
		AddChat(CurrentSelectedGroup->ChatBlockUI->GetTeamID(), TimeStamp, MyName, Content);
	}
	EditBox->SetText(FText::GetEmpty());

	// [전송 로직 분기]
	if (CurrentSelectedGroup->bIsChatbotRoom)
	{
		// [챗봇] 웹소켓 Query 전송
		if (WebSocketSystem && WebSocketSystem->IsConnected())
		{
			FWSRequest_Query QueryReq;
			QueryReq.sessionId = FString::FromInt(MyID);
			QueryReq.query = Content;
			WebSocketSystem->SendStructMessage(QueryReq);
		}
		else
		{
			// [예외처리] 연결 끊김 -> 챗봇 말풍선으로 경고 출력
			AddBotChat(TEXT("무물이와 연결되어 있지 않습니다. 잠시 후 다시 시도해주세요."));
		}
	}
	else
	{
		// [일반] HTTP 전송 및 RPC
		// ChatBlockUI 유효성 재확인
		if (CurrentSelectedGroup->ChatBlockUI)
		{
			FString TeamID = CurrentSelectedGroup->ChatBlockUI->GetTeamID();
            
			// TeamUserIDs 가져오기
			TArray<int32> UserIDs;
			CurrentSelectedGroup->ChatBlockUI->GetTeamUsers().GetKeys(UserIDs);

			if (HttpSystem)
			{
				HttpSystem->SendChatMessageRequest(TeamID, MyID, Content, TimeStamp);
			}
            
			if (ACuteAlienController* PC = Cast<ACuteAlienController>(GetOwningPlayer()))
			{
				PC->Server_RequestChat(TeamID, UserIDs, TimeStamp, MyName, Content);
			}
		}
	}
    
	EditBox->SetFocus();
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
