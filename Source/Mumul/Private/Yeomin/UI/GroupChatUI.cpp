// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/GroupChatUI.h"

#include "HttpNetworkSubsystem.h"
#include "Components/Border.h"
#include "WebSocketSubsystem.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
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
#include "Yeomin/UI/BaseUI/BaseText.h"
#include "MumulGameInstance.h" // 필수
#include "Components/Image.h"
#include "Components/ScaleBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Library/MathLibrary.h"
#include "Yeomin/Data/IMGManager.h"
#include "Yeomin/UI/BotChatMessageBlockUI.h"

void UGroupChatUI::NativeConstruct()
{
	Super::NativeConstruct();

	IMGManager = NewObject<UIMGManager>(this, UIMGManager::StaticClass());

	ChatEnter->OnPressed.AddDynamic(this, &UGroupChatUI::OnTextBoxCommitted);
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
		HttpSystem->OnChatHistoryResponse.AddDynamic(this, &UGroupChatUI::OnServerChatHistoryResponse);
	}

	WebSocketSystem = GetGameInstance()->GetSubsystem<UWebSocketSubsystem>();
	if (WebSocketSystem)
	{
		// 1. 학습 챗봇 (Learning)
		WebSocketSystem->OnLearningChatStarted.AddDynamic(this, &UGroupChatUI::OnLearningChatStarted);
		WebSocketSystem->OnLearningChatAnswer.AddDynamic(this, &UGroupChatUI::OnLearningChatAnswer);
		WebSocketSystem->OnLearningChatEnded.AddDynamic(this, &UGroupChatUI::OnLearningChatEnded);

		// 2. 회의 도우미 (Meeting)
		WebSocketSystem->OnMeetingChatStarted.AddDynamic(this, &UGroupChatUI::OnMeetingChatStarted);
		WebSocketSystem->OnMeetingChatAnswer.AddDynamic(this, &UGroupChatUI::OnMeetingChatAnswer);
		WebSocketSystem->OnMeetingChatEnded.AddDynamic(this, &UGroupChatUI::OnMeetingChatEnded);
	}

	if (InviteBtn && NaNumiScaleBox)
	{
		InviteBtn->SetVisibility(ESlateVisibility::Collapsed);
		NaNumiScaleBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (QuestionBtn)
	{
		QuestionBtn->OnClicked.AddDynamic(this, &UGroupChatUI::OnClickQuestionBtn);
	}

	// [신규] 챗봇 방 생성 및 상단 배치
	InitChatbotRoom();
}

void UGroupChatUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bAnimating)
		return;

	Elapsed += InDeltaTime;
	float Alpha = FMath::Clamp(Elapsed / Duration, 0.f, 1.f);

	float Eased = UMathLibrary::EaseOutQuint(Alpha);

	AlignmentVal = FMath::Lerp(StartVal, TargetVal, Eased);

	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(GroupChatBorder->Slot))
	{
		CanvasPanelSlot->SetAlignment(FVector2D(AlignmentVal, 0.5f));
	}

	if (Alpha >= 1.f)
	{
		bAnimating = false;
	}
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
	int32 UserID = GI ? GI->PlayerUniqueID : 0;

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

	if (bIsMeetingChatbotActive)
	{
		if (WebSocketSystem && WebSocketSystem->IsConnected())
		{
			FWSRequest_EndChat EndReq;
			EndReq.sessionId = UserID;
			WebSocketSystem->SendStructMessage(EndReq);
			WebSocketSystem->Close();
		}
		bIsMeetingChatbotActive = false;
		if (InviteBtn && NaNumiScaleBox)
		{
			InviteBtn->SetVisibility(ESlateVisibility::Collapsed);
			NaNumiScaleBox->SetVisibility(ESlateVisibility::Collapsed);
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
		if (InviteBtn && NaNumiScaleBox)
		{
			ChatbotIcon->SetIconIMG(MumuLeeOnIMG);
			InviteBtn->SetVisibility(ESlateVisibility::Collapsed);
			NaNumiScaleBox->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (SelectedIcon->ChatBlockUI)
		{
			SelectedIcon->ChatBlockUI->ChatScrollBox->ClearChildren();
		}

		if (GI && HttpSystem)
		{
			HttpSystem->SendChatHistoryRequest(GI->PlayerUniqueID);
		}

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
			WebSocketSystem->Connect(TEXT("learning_chatbot"));

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
		if (InviteBtn && QuestionBtn && NaNumiScaleBox)
		{
			ChatbotIcon->SetIconIMG(MumuLeeOffIMG);
			InviteBtn->SetVisibility(ESlateVisibility::Visible);
			NaNumiScaleBox->SetVisibility(ESlateVisibility::Visible);

			// 방을 옮겼으므로 AI 도우미는 꺼진 상태로 초기화
			bIsMeetingChatbotActive = false;
			UpdateQuestionButtonState();
		}

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

// [1] 학습 챗봇 핸들러 (Chatbot_Room 전용)
void UGroupChatUI::OnLearningChatStarted(FString Message)
{
	// 학습 챗봇 방을 보고 있을 때만 메시지 표시
	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
	{
		AddBotChat(Message);
	}
}

void UGroupChatUI::OnLearningChatAnswer(FString Answer)
{
	// 학습 챗봇은 무조건 챗봇 전용 방에만 뜹니다.
	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
	{
		AddBotChat(Answer);
	}
}

void UGroupChatUI::OnLearningChatEnded(FString Message)
{
	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
	{
		AddBotChat(Message);
	}
}


// [2] 회의 도우미 핸들러 (일반 채팅방 전용)
void UGroupChatUI::OnMeetingChatStarted(FString Message, FString GroupId, FString UserName)
{
	// 내가 보고 있는 방이, 도우미가 시작된 그 방인지 확인
	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
	{
		if (CurrentSelectedGroup->ChatBlockUI->GetTeamID() == GroupId)
		{
			// 예: "[알림] 홍길동님이 회의 도우미를 시작했습니다." 같은 시스템 메시지로 띄울 수도 있음
			AddBotChat(FString::Printf(TEXT("[알림] %s"), *Message));
		}
	}
}

void UGroupChatUI::OnMeetingChatAnswer(FString Answer, FString GroupId)
{
	// 답변이 도착한 방(GroupId)이 현재 보고 있는 방과 일치하는지 확인
	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
	{
		if (CurrentSelectedGroup->ChatBlockUI->GetTeamID() == GroupId)
		{
			AddBotChat(Answer);
		}
	}
}

void UGroupChatUI::OnMeetingChatEnded(FString Message, FString GroupId)
{
	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
	{
		if (CurrentSelectedGroup->ChatBlockUI->GetTeamID() == GroupId)
		{
			AddBotChat(FString::Printf(TEXT("[알림] %s"), *Message));

			// 만약 내가 켠 사람이라면 버튼 상태도 꺼줌
			bIsMeetingChatbotActive = false;
			UpdateQuestionButtonState();
		}
	}
}

// void UGroupChatUI::OnAIChatStarted(FString Message)
// {
// 	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
// 	{
// 		AddBotChat(Message); // [수정] AddChat -> AddBotChat
// 	}
// }
//
// void UGroupChatUI::OnAIChatAnswer(FString Answer, FString GroupId)
// {
// 	// 1. 학습 챗봇 (GroupId가 없거나 특정 ID)
// 	if (GroupId.IsEmpty() || GroupId == TEXT("Chatbot_Room"))
// 	{
// 		// 현재 보고 있는 방이 챗봇 방이면 바로 추가
// 		if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
// 		{
// 			AddBotChat(Answer);
// 		}
// 		// 안 보고 있다면? (나중에 볼 수 있게 데이터에만 추가하거나 알림)
// 	}
// 	// 2. 회의 도우미 (GroupId가 있음)
// 	else
// 	{
// 		// 현재 보고 있는 방이 그 방인가?
// 		if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
// 		{
// 			if (CurrentSelectedGroup->ChatBlockUI->GetTeamID() == GroupId)
// 			{
// 				AddBotChat(Answer); // 현재 화면에 추가
// 			}
// 		}
//
// 		// (선택) 안 보고 있더라도 그 방의 ChatBlockUI를 찾아서 추가해줘야 함.
// 		// GroupScrollBox를 순회하며 GroupId가 일치하는 아이콘 찾기 -> 그 아이콘의 ChatBlockUI에 추가
// 	}
// }

void UGroupChatUI::InitChatbotRoom()
{
	if (!GroupIconUIClass || !ChatBlockUIClass) return;

	// 1. 챗봇용 아이콘 생성
	ChatbotIcon = CreateWidget<UGroupIconUI>(GetWorld(), GroupIconUIClass);
	if (ChatbotIcon)
	{
		ChatbotIcon->InitParentUI(this);
		ChatbotIcon->bIsChatbotRoom = true; // 챗봇 방임을 표시

		// 2. 챗봇용 채팅 블록(내용창) 생성
		UChatBlockUI* ChatbotBlock = CreateWidget<UChatBlockUI>(GetWorld(), ChatBlockUIClass);
		ChatbotBlock->SetTeamID(TEXT("Chatbot_Room"));
		ChatbotBlock->SetTeamName(TEXT("무엇이든 물어보세요!"));

		ChatbotIcon->ChatBlockUI = ChatbotBlock; // 아이콘에 연결
		ChatbotIcon->SetIconIMG(MumuLeeOffIMG);

		// [수정] 메시지는 여기서 넣지 않음! (비워둠)

		// 3. [수정] 스크롤박스에 추가 (InsertAt 대신 AddChild 사용)
		MumuLeeBox->AddChild(ChatbotIcon);
	}
}

void UGroupChatUI::OnServerChatHistoryResponse(bool bSuccess, FString Message)
{
	// 현재 챗봇 방을 보고 있지 않다면 무시
	if (!CurrentSelectedGroup || !CurrentSelectedGroup->bIsChatbotRoom) return;

	if (bSuccess)
	{
		FChatHistoryResponse HistoryData;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Message, &HistoryData, 0, 0))
		{
			UE_LOG(LogTemp, Log, TEXT("[ChatHistory] Loaded %d messages"), HistoryData.messages.Num());

			UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
			FString MyName = GI ? GI->PlayerName : TEXT("Me");

			// 메시지 순회하며 UI 추가
			for (const FChatHistoryMessage& Msg : HistoryData.messages)
			{
				FString ParsedTime = ParseTimeFromISO8601(Msg.created_at);

				if (Msg.role == TEXT("user"))
				{
					// 내 질문 -> 일반 말풍선 (AddChat)
					// (TeamID는 현재 챗봇방 ID 사용)
					AddChat(CurrentSelectedGroup->ChatBlockUI->GetTeamID(), ParsedTime, MyName, Msg.content);
				}
				else if (Msg.role == TEXT("assistant"))
				{
					// 챗봇 답변 -> 봇 말풍선 (AddBotChat 사용은 주의: AddBotChat은 현재 시간 쓰므로 수정 필요)
					// 기존 AddBotChat은 현재 시간을 찍으므로, 시간을 인자로 받는 버전으로 오버로딩하거나 직접 구현

					// 여기서는 직접 구현 예시 (AddBotChat 로직 활용)
					if (BotChatMessageBlockUIClass && CurrentSelectedGroup->ChatBlockUI)
					{
						UChatMessageBlockUI* BotChat = CreateWidget<UChatMessageBlockUI>(
							GetWorld(), BotChatMessageBlockUIClass);
						if (BotChat)
						{
							CurrentSelectedGroup->ChatBlockUI->ChatScrollBox->AddChild(BotChat);
							BotChat->SetContent(ParsedTime, TEXT("무물이"), Msg.content);
						}
					}
				}
			}

			// 스크롤 맨 아래로
			if (CurrentSelectedGroup->ChatBlockUI)
			{
				CurrentSelectedGroup->ChatBlockUI->ChatScrollBox->ScrollToEnd();
			}
		}
	}
	else
	{
		// 실패 시 (404 등) -> 대화 내용이 없으면 환영 메시지 띄우기
		// (기존 SelectGroupChat에 있던 환영 메시지 타이머 로직이 여기서 자연스럽게 대체될 수 있음)
		UE_LOG(LogTemp, Warning, TEXT("[ChatHistory] Failed or Empty: %s"), *Message);
	}
}

FString UGroupChatUI::ParseTimeFromISO8601(const FString& IsoString)
{
	// 예: "2025-12-05T10:25:53.093000" -> "10:25"
	FDateTime DateTime;
	if (FDateTime::ParseIso8601(*IsoString, DateTime))
	{
		return DateTime.ToString(TEXT("%H:%M"));
	}
	return TEXT(""); // 파싱 실패 시 공란
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
			if (WebSocketSystem->CurrentChatbotType == EWebSocketChatbotType::Learning)
				BotChat->SetContent(TimeStamp, TEXT("무물이"), Message);
			else if (WebSocketSystem->CurrentChatbotType == EWebSocketChatbotType::Meeting)
				BotChat->SetContent(TimeStamp, TEXT("나눔이"), Message);


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

void UGroupChatUI::OnTextBoxCommitted()
{
	FText Text = EditBox->GetText();

	if (Text.IsEmpty()) return;
	if (!CurrentSelectedGroup) return;

	// 기본 변수 선언
	FString Content = Text.ToString();
	FString TimeStamp = FDateTime::Now().ToString(TEXT("%H:%M"));

	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	FString MyName = GI ? GI->PlayerName : TEXT("Me");
	int32 MyID = GI ? GI->PlayerUniqueID : 0;


	// [핵심 수정: 2. 텍스트 박스 내용을 비웁니다.]

	// [공통] 내 화면에 메시지 즉시 추가
	// if (CurrentSelectedGroup->ChatBlockUI)
	// {
	// 	AddChat(CurrentSelectedGroup->ChatBlockUI->GetTeamID(), TimeStamp, MyName, Content);
	// }
	// EditBox->SetText(FText::GetEmpty());

	// [전송 로직 분기]
	if (CurrentSelectedGroup->bIsChatbotRoom)
	{
		AddChat(CurrentSelectedGroup->ChatBlockUI->GetTeamID(), TimeStamp, MyName, Content);
		// === Case A: 학습 챗봇 방 (개인용) ===
		if (WebSocketSystem && WebSocketSystem->IsConnected())
		{
			FWSRequest_Query QueryReq;
			QueryReq.sessionId = MyID; // 학습 챗봇은 sessionId = userId
			QueryReq.userId = MyID;
			QueryReq.query = Content;
			WebSocketSystem->SendStructMessage(QueryReq);
		}
		else
		{
			AddBotChat(TEXT("무물이와 연결되어 있지 않습니다. 잠시 후 다시 시도해주세요."));
		}
	}
	else
	{
		// === Case B: 일반 그룹 채팅방 (공용) ===
		if (CurrentSelectedGroup->ChatBlockUI)
		{
			FString TeamID = CurrentSelectedGroup->ChatBlockUI->GetTeamID();

			// 1. [DB 저장] HTTP 전송
			if (HttpSystem)
			{
				HttpSystem->SendChatMessageRequest(TeamID, MyID, Content, TimeStamp);
			}

			// 2. [채팅 공유] RPC 전송 (팀원들에게 내 질문이 보이게 함)
			TArray<int32> UserIDs;
			CurrentSelectedGroup->ChatBlockUI->GetTeamUsers().GetKeys(UserIDs);

			if (ACuteAlienController* PC = Cast<ACuteAlienController>(GetOwningPlayer()))
			{
				PC->Server_RequestChat(TeamID, UserIDs, TimeStamp, MyName, Content);
			}

			// 3. [AI 질문] 도우미가 켜져 있다면 웹소켓으로도 전송
			if (bIsMeetingChatbotActive)
			{
				if (WebSocketSystem && WebSocketSystem->IsConnected())
				{
					FWSRequest_MeetingQuery QueryReq;
					QueryReq.groupId = TeamID; // 회의 도우미는 groupId 기준
					QueryReq.userId = MyID;
					QueryReq.userName = MyName;
					QueryReq.query = Content;

					WebSocketSystem->SendStructMessage(QueryReq);
					UE_LOG(LogTemp, Log, TEXT("[MeetingBot] Query Sent: %s"), *Content);
				}
				else
				{
					// 연결 끊김 안내 및 토글 해제
					AddBotChat(TEXT("회의 도우미와 연결이 끊겼습니다."));

					// [수정] 강제 OFF 처리
					bIsMeetingChatbotActive = false;
					UpdateQuestionButtonState();
				}
			}
		}
	}
	FSlateApplication::Get().SetKeyboardFocus(ChatEnter->TakeWidget());
	EditBox->SetText(FText::FromString(TEXT("")));
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
				UTexture2D* TeamIconIMG = IMGManager->GetImageByTeamID(TeamChat.teamChatId);
				GroupIconUI->SetIconIMG(TeamIconIMG);

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
	GroupNameTitle->BaseText->SetText(FText::FromString(GroupName));
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

void UGroupChatUI::ToggleGroupChatAlignment()
{
	// Change Toggle State
	bIsToggled = !bIsToggled;

	// 버튼 이미지 처리
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D(56.f, 84.f);
	Brush.SetResourceObject(bIsToggled ? RightIMG : LeftIMG);

	FButtonStyle Style;
	Style.Normal = Brush;
	Style.Hovered = Brush;
	Style.Pressed = Brush;
	ToggleVisibilityBtn->SetStyle(Style);

	// 애니메이션 시작
	StartVal = AlignmentVal;
	TargetVal = bIsToggled ? 1.f : 0.1968f;
	Elapsed = 0.f;
	bAnimating = true;
}

void UGroupChatUI::OnToggleVisibilityBtn()
{
	ToggleGroupChatAlignment();
	
	CreateGroupChatUI->RefreshJoinedPlayerList();
	
	// Get TeamChatList
	AMumulPlayerState* PS = Cast<AMumulPlayerState>(GetOwningPlayerState());
	HttpSystem->SendTeamChatListRequest(PS->PS_UserIndex);
	
	// test: create team
	if (bITestCreateTeamChat)
	{
		ACuteAlienController* PC = Cast<ACuteAlienController>(GetOwningPlayer());
		if (PC->IsLocalController())
		{
			TArray<int32> testList = {10, 11};
			TArray<FTeamUser> TeamUserIDs;
			PC->Server_CreateGroupChatUI(testList, FString(TEXT("team01")), FString(TEXT("Test")),
			                             TeamUserIDs);
		}
	}
}

void UGroupChatUI::OnAICheckStateChanged(bool bIsChecked)
{
	// 일반 방이 아니면 무시
	if (!CurrentSelectedGroup || CurrentSelectedGroup->bIsChatbotRoom) return;
	UE_LOG(LogTemp, Log, TEXT("[AI Chat Mode : %s"), bIsChecked ? TEXT("On") : TEXT("Off"));

	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	if (!GI) return;

	int32 MyID = GI->PlayerUniqueID;
	FString MyName = GI->PlayerName;
	FString GroupID = GetCurrentTeamID();

	bIsMeetingChatbotActive = bIsChecked;
	UpdateQuestionButtonState();

	if (bIsChecked)
	{
		// [ON] 웹소켓 연결 및 Start
		if (WebSocketSystem)
		{
			// 엔드포인트: meeting_chatbot
			WebSocketSystem->Connect(TEXT("meeting_chatbot"));

			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, [this, MyID, MyName, GroupID]()
			{
				if (WebSocketSystem && WebSocketSystem->IsConnected())
				{
					FWSRequest_MeetingStart StartReq;
					StartReq.groupId = GroupID;
					StartReq.userId = MyID;
					StartReq.userName = MyName;

					WebSocketSystem->SendStructMessage(StartReq);
					UE_LOG(LogTemp, Log, TEXT("[MeetingBot] Connected & Started for Group: %s"), *GroupID);
				}
			}, 0.5f, false);
		}
	}
	else
	{
		// [OFF] 종료 및 연결 해제
		if (WebSocketSystem && WebSocketSystem->IsConnected())
		{
			FWSRequest_MeetingEnd EndReq;
			EndReq.groupId = GroupID;
			EndReq.userId = MyID;

			WebSocketSystem->SendStructMessage(EndReq);
			WebSocketSystem->Close();

			UE_LOG(LogTemp, Log, TEXT("[MeetingBot] Disconnected"));
		}
	}
}

void UGroupChatUI::OnClickQuestionBtn()
{
	// 토글 로직: 현재 상태 반전
	bool bNewState = !bIsMeetingChatbotActive;

	UE_LOG(LogTemp, Log, TEXT("[AI Chat Mode] State : %s"), bNewState ? TEXT("On") : TEXT("Off"));

	// 기존 로직 재활용 (OnAICheckStateChanged 내용을 그대로 쓰거나 호출)
	OnAICheckStateChanged(bNewState);

	// 버튼 스타일 업데이트 (OnAICheckStateChanged 내부에서 호출해도 됨)
	UpdateQuestionButtonState();
}

void UGroupChatUI::UpdateQuestionButtonState()
{
	if (!QuestionBtn) return;

	if (bIsMeetingChatbotActive)
	{
		// [ON 상태] 예: 초록색 배경
		NaNumiIMG->SetBrushFromTexture(NaNumiOnIMG);
		QuestionBtn->SetBackgroundColor(FLinearColor(0.2f, 1.0f, 0.2f, 1.0f));
	}
	else
	{
		// [OFF 상태] 예: 기본색 (흰색/회색)
		NaNumiIMG->SetBrushFromTexture(NaNumiOffIMG);
		QuestionBtn->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	}
}

FString UGroupChatUI::GetCurrentTeamID() const
{
	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
	{
		return CurrentSelectedGroup->ChatBlockUI->GetTeamID();
	}
	return TEXT("");
}

FString UGroupChatUI::GetCurrentTeamName() const
{
	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
	{
		return CurrentSelectedGroup->ChatBlockUI->GetTeamName();
	}
	return TEXT("");
}

void UGroupChatUI::UpdateDot()
{
	// Repeat 1 → 2 → 3
	DotCount = (DotCount % 3) + 1;

	FString Dots = FString::ChrN(DotCount, TEXT('.'));
	FString Text = FString::Printf(TEXT("기록중%s"), *Dots);

	if (RecordText1)
	{
		RecordText1->BaseText->SetText(FText::FromString(Text));
	}
}

void UGroupChatUI::OnRecordBtnState(bool bIsOn)
{
	if (bIsOn == true)
	{
		RecordIMG->SetBrushFromTexture(RecordIMGs[1]);
		RecordText0->BaseText->SetText(FText::FromString(TEXT("나눔이가")));

		GetWorld()->GetTimerManager().SetTimer(
			DotTimer,
			this,
			&UGroupChatUI::UpdateDot,
			1.1f,
			true
		);
	}
	else if (bIsOn == false)
	{
		GetWorld()->GetTimerManager().ClearTimer(DotTimer);
		RecordIMG->SetBrushFromTexture(RecordIMGs[0]);
		RecordText0->BaseText->SetText(FText::FromString(TEXT("나눔이로")));
		RecordText1->BaseText->SetText(FText::FromString(TEXT("기록하기")));
	}
}
