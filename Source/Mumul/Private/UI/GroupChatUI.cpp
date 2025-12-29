// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GroupChatUI.h"

#include "Animation/WidgetAnimation.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Network/WebSocketSubsystem.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Player/MumulPlayerState.h"
#include "Network/NetworkStructs.h"
#include "Player/CuteAlienController.h"
#include "UI/ChatBlockUI.h"
#include "UI/ChatMessageBlockUI.h"
#include "UI/CreateGroupChatUI.h"
#include "UI/GroupIconUI.h"
#include "UI/InvitationUI.h"
#include "UI/BaseUI/BaseText.h"
#include "Base/MumulGameInstance.h" // 필수
#include "Base/MumulGameState.h"
#include "Components/Image.h"
#include "Components/VerticalBoxSlot.h"
#include "Data/AudioManager.h"
#include "Library/MathLibrary.h"
#include "Data/IMGManager.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Component/PlayerChatComponent.h"
#include "Player/Component/PlayerMeetingManagerComponent.h"
#include "Save/MapDataSaveGame.h"
#include "UI/BotChatMessageBlockUI.h"
#include "UI/PlayerUI.h"

void UGroupChatUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWidget* RootWidget = GetRootWidget())
	{
		// 배경 투명 영역 클릭 통과 설정
		RootWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	AudioManager = GetGameInstance()->GetSubsystem<UAudioManager>();
	IMGManager = NewObject<UIMGManager>(this, UIMGManager::StaticClass());

	ChatEnter->OnPressed.AddDynamic(this, &UGroupChatUI::OnTextBoxCommitted);
	AddGroupBtn->OnPressed.AddDynamic(this, &UGroupChatUI::ToggleCreateGroupChatUI);

	CreateGroupChatUI = CreateWidget<UCreateGroupChatUI>(this, CreateGroupChatUIClass);
	CreateGroupChatUI->InitParentUI(this);
	CreateGroupChatBox->AddChild(CreateGroupChatUI);

	InvitationUI = CreateWidget<UInvitationUI>(this, InvitationUIClass);
	InvitationBox->AddChild(InvitationUI);
	InvitationBox->SetVisibility(ESlateVisibility::Collapsed);

	InviteBtn->OnPressed.AddDynamic(this, &UGroupChatUI::ToggleInvitationUI);
	ToggleVisibilityBtn->OnPressed.AddDynamic(this, &UGroupChatUI::OnToggleVisibilityBtn);

	// Register HTTP Response callback function
	HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>();
	if (HttpSystem)
	{
		HttpSystem->OnTeamChatListResponse.AddDynamic(this, &UGroupChatUI::OnServerTeamChatListResponse);
		HttpSystem->OnChatMessageResponse.AddDynamic(this, &UGroupChatUI::OnServerChatMessageResponse);
		HttpSystem->OnChatHistoryResponse.AddDynamic(this, &UGroupChatUI::OnServerChatHistoryResponse);
		HttpSystem->OnTeamChatMessageResponse.AddDynamic(this, &UGroupChatUI::OnServerTeamChatMessageResponse);
	}

	WebSocketSystem = GetGameInstance()->GetSubsystem<UWebSocketSubsystem>();
	if (WebSocketSystem)
	{
		// 1. 학습 챗봇 (Learning)
		WebSocketSystem->OnLearningChatStarted.AddDynamic(this, &UGroupChatUI::OnLearningChatStarted);
		WebSocketSystem->OnLearningAnswer.AddDynamic(this, &UGroupChatUI::OnLearningChatAnswer); // 이름 변경됨
		WebSocketSystem->OnLearningChatEnded.AddDynamic(this, &UGroupChatUI::OnLearningChatEnded);

		// 2. 회의 도우미 (Meeting)
		WebSocketSystem->OnMeetingChatStarted.AddDynamic(this, &UGroupChatUI::OnMeetingChatStarted);
		WebSocketSystem->OnMeetingAnswer.AddDynamic(this, &UGroupChatUI::OnMeetingAnswer); // 이름 변경됨
		WebSocketSystem->OnMeetingChatEnded.AddDynamic(this, &UGroupChatUI::OnMeetingChatEnded);
       
		// [권장] UI가 켜질 때 웹소켓이 연결되어 있지 않다면 연결 시도 (통합 연결)
		if (!WebSocketSystem->IsConnected())
		{
			WebSocketSystem->Connect();
		}
	}

	if (InviteBtn && NaNumiSizeBox && MumuLeeSizeBox)
	{
		InviteBtn->SetVisibility(ESlateVisibility::Collapsed);
		MumuLeeSizeBox->SetVisibility(ESlateVisibility::Collapsed);
		NaNumiSizeBox->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (QuestionBtn)
	{
		QuestionBtn->OnClicked.AddDynamic(this, &UGroupChatUI::OnClickQuestionBtn);
	}

	if (BeginnerBtn && IntermediateBtn && AdvancedBtn)
	{
		BeginnerBtn->OnClicked.AddDynamic(this, &UGroupChatUI::OnBeginnerClicked);
		IntermediateBtn->OnClicked.AddDynamic(this, &UGroupChatUI::OnNormalClicked);
		AdvancedBtn->OnClicked.AddDynamic(this, &UGroupChatUI::OnAdvancedClicked);
	}

	// [신규] 챗봇 방 생성 및 상단 배치
	InitChatbotRoom();

	// 테스트용 팀채팅
	UGroupIconUI* GroupIconUI = CreateWidget<UGroupIconUI>(GetWorld(), GroupIconUIClass);
	AddGroupIcon(GroupIconUI);
	GroupIconUI->InitParentUI(this);
	GroupIconUI->ChatBlockUI->SetTeamID(TEXT("testteam"));
	GroupIconUI->ChatBlockUI->SetTeamName(TEXT("테스트용"));
	GroupIconUI->ChatBlockUI->AddTeamUser(1220, TEXT("테스트 유저"));
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

	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(GroupChatBox->Slot))
	{
		CanvasPanelSlot->SetAlignment(FVector2D(AlignmentVal, 0.5f));
	}

	if (Alpha >= 1.f)
	{
		bAnimating = false;
	}
}

void UGroupChatUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// Init CreateGroupUI position
	PlayAnimation(CreateGroupUI_Slide, CreateGroupUI_Slide->GetEndTime(), 1, EUMGSequencePlayMode::Reverse);
}

void UGroupChatUI::ToggleVisibility(UWidget* Widget)
{
	const bool bIsVisible = (Widget->GetVisibility() == ESlateVisibility::Visible);
	Widget->SetVisibility(bIsVisible ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
}

// void UGroupChatUI::SelectGroupChat(class UGroupIconUI* SelectedIcon)
// {
// 	if (!SelectedIcon) return;
// 	if (CurrentSelectedGroup == SelectedIcon) return;
//
// 	if (ACuteAlienController* PC = Cast<ACuteAlienController>(GetOwningPlayer()))
// 	{
// 		// MeetingComp가 있고, 세션 ID가 있다면 회의 중
// 		if (PC->MeetingComp && !PC->MeetingComp->CurrentMeetingSessionID.IsEmpty())
// 		{
// 			AddBotChat(TEXT("회의 중에는 다른 채팅방으로 이동할 수 없습니다."));
// 			return;
// 		}
// 	}
//
// 	if (CurrentSelectedGroup)
// 	{
// 		CurrentSelectedGroup->SetHighlight(false);
// 	}
// 	
// 	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
// 	int32 UserID = GI ? GI->PlayerUniqueID : 0;
//
// 	// 1. 이전 방 정리 (챗봇 방에서 나가는 경우)
// 	
// 	// if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
// 	// {
// 	// 	if (WebSocketSystem && WebSocketSystem->IsConnected())
// 	// 	{
// 	// 		FWSRequest_EndChat EndReq;
// 	// 		EndReq.sessionId = UserID;
// 	// 		WebSocketSystem->SendStructMessage(EndReq);
// 	// 		WebSocketSystem->Close();
// 	// 	}
// 	// }
//
// 	if (bIsMeetingChatbotActive)
// 	{
// 		if (WebSocketSystem && WebSocketSystem->IsConnected())
// 		{
// 			FWSRequest_EndChat EndReq;
// 			EndReq.sessionId = UserID;
// 			WebSocketSystem->SendStructMessage(EndReq);
// 			WebSocketSystem->Close();
// 		}
// 		bIsMeetingChatbotActive = false;
// 		if (InviteBtn && NaNumiSizeBox && MumuLeeSizeBox)
// 		{
// 			MumuLeeSizeBox->SetVisibility(ESlateVisibility::Visible);
//
// 			InviteBtn->SetVisibility(ESlateVisibility::Collapsed);
// 			NaNumiSizeBox->SetVisibility(ESlateVisibility::Collapsed);
// 		}
// 	}
//
// 	// 2. UI 교체
// 	RemoveChatBlock();
// 	if (SelectedIcon->ChatBlockUI)
// 	{
// 		AddChatBlock(SelectedIcon->ChatBlockUI);
// 		SetGroupNameTitle(SelectedIcon->ChatBlockUI->GetTeamName());
// 	}
//
// 	CurrentSelectedGroup = SelectedIcon;
//
// 	if (CurrentSelectedGroup)
// 	{
// 		CurrentSelectedGroup->SetHighlight(true);
// 	}
//
// 	// 3. 새 방 진입 처리
// 	if (SelectedIcon->bIsChatbotRoom)
// 	{
// 		if (InviteBtn && NaNumiSizeBox && MumuLeeSizeBox)
// 		{
// 			ChatbotIcon->SetIconIMG(MumuLeeOnIMG);
// 			MumuLeeSizeBox->SetVisibility(ESlateVisibility::Visible);
//
// 			InviteBtn->SetVisibility(ESlateVisibility::Collapsed);
// 			NaNumiSizeBox->SetVisibility(ESlateVisibility::Collapsed);
// 		}
// 		if (SelectedIcon->ChatBlockUI)
// 		{
// 			SelectedIcon->ChatBlockUI->ChatScrollBox->ClearChildren();
// 		}
//
// 		if (GI && HttpSystem)
// 		{
// 			HttpSystem->SendChatHistoryRequest(GI->PlayerUniqueID);
// 		}
//
// 		// [챗봇 방] 웹소켓 연결 시도
// 		if (SelectedIcon->ChatBlockUI && SelectedIcon->ChatBlockUI->ChatScrollBox->GetChildrenCount() == 0)
// 		{
// 			FTimerHandle WelcomeHandle;
// 			GetWorld()->GetTimerManager().SetTimer(WelcomeHandle, [this]()
// 			{
// 				AddBotChat(TEXT("안녕하세요! 무엇을 도와드릴까요?"));
// 			}, 0.1f, false);
// 		}
//
// 		if (WebSocketSystem)
// 		{
// 			WebSocketSystem->Connect(TEXT("learning_chatbot"));
//
// 			FTimerHandle ConnectTimerHandle;
// 			GetWorld()->GetTimerManager().SetTimer(ConnectTimerHandle, [this, UserID]()
// 			{
// 				if (!this || !WebSocketSystem) return;
//
// 				if (WebSocketSystem->IsConnected())
// 				{
// 					FWSRequest_StartChat StartReq;
// 					StartReq.sessionId = UserID;
// 					StartReq.userId = UserID;
// 					WebSocketSystem->SendStructMessage(StartReq);
// 				}
// 				else
// 				{
// 					// 연결 대기 중...
// 				}
// 			}, 0.5f, false);
// 		}
// 	}
// 	else
// 	{
// 		if (InviteBtn && QuestionBtn && NaNumiSizeBox && MumuLeeSizeBox)
// 		{
// 			ChatbotIcon->SetIconIMG(MumuLeeOffIMG);
// 			MumuLeeSizeBox->SetVisibility(ESlateVisibility::Collapsed);
//
// 			InviteBtn->SetVisibility(ESlateVisibility::Visible);
// 			NaNumiSizeBox->SetVisibility(ESlateVisibility::Visible);
//
// 			// 방을 옮겼으므로 AI 도우미는 꺼진 상태로 초기화
// 			bIsMeetingChatbotActive = false;
// 			UpdateQuestionButtonState();
// 		}
//
// 		// [일반 방] 로직 복원
//
// 		// 1) HTTP로 지난 대화 내역 불러오기
// 		if (HttpSystem)
// 		{
// 			HttpSystem->SendTeamChatMessageRequest(SelectedIcon->ChatBlockUI->GetTeamID());
// 		}
//
// 		// 2) 보이스 채널 변경 (주석 해제 및 복원)
// 		if (AMumulPlayerState* PS = Cast<AMumulPlayerState>(GetOwningPlayerState()))
// 		{
// 			// TeamID(String)를 int32로 변환
// 			FString TargetChannelID = SelectedIcon->ChatBlockUI->GetTeamID();
//
// 			if (PS->bIsNearByCampFire)
// 			{
// 				PS->Server_SetVoiceChannelID(TargetChannelID);
// 			}
// 			else
// 			{
// 				PS->WaitingChannelID = TargetChannelID;
// 			}
//
// 			UE_LOG(LogTemp, Log, TEXT("[UI] Switched Voice Channel to: %s"), *TargetChannelID);
// 		}
// 	}
// }

void UGroupChatUI::SelectGroupChat(class UGroupIconUI* SelectedIcon)
{
    // 1. 유효성 검사 및 중복 선택 방지
    if (!SelectedIcon) return;
    if (CurrentSelectedGroup == SelectedIcon) return;

    // 2. 회의 진행 중 이동 제한 체크
    if (ACuteAlienController* PC = Cast<ACuteAlienController>(GetOwningPlayer()))
    {
        // MeetingComp가 있고, 현재 회의 세션 ID가 존재한다면 이동 불가
        if (PC->MeetingComp && !PC->MeetingComp->CurrentMeetingSessionID.IsEmpty())
        {
            AddBotChat(TEXT("회의 중에는 다른 채팅방으로 이동할 수 없습니다."));
            return;
        }
    }

    // 3. 기존 선택된 그룹 아이콘 하이라이트 해제
    if (CurrentSelectedGroup)
    {
        CurrentSelectedGroup->SetHighlight(false);
    }
    
    // 유저 정보 가져오기 (세션 ID용)
    UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
    int32 UserID = GI ? GI->PlayerUniqueID : 0;

    // =================================================================================
    // 4. [이전 방 정리] 방을 나갈 때 처리 (웹소켓 세션 종료 요청)
    // =================================================================================
    
    // Case A: 이전에 '학습 챗봇 방(무물이)'에 있었다면 -> 학습 세션 종료
    if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
    {
        if (WebSocketSystem && WebSocketSystem->IsConnected())
        {
            // [수정] 소켓을 끊지 않고 '종료 패킷'만 전송
            // 학습 챗봇은 SessionId와 UserId를 동일하게 사용한다고 가정
            WebSocketSystem->EndLearningChat(UserID, UserID);
        }
    }

    // Case B: 이전에 '일반 방'에서 '회의 도우미(나눔이)'를 켜두었다면 -> 회의 세션 종료
    if (bIsMeetingChatbotActive)
    {
        if (WebSocketSystem && WebSocketSystem->IsConnected())
        {
            // 이전에 보고 있던 방의 GroupID가 필요함
            if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
            {
                FString OldGroupID = CurrentSelectedGroup->ChatBlockUI->GetTeamID();
                // [수정] 소켓을 끊지 않고 '종료 패킷'만 전송
                WebSocketSystem->EndMeetingChat(OldGroupID);
            }
        }
        
        // 상태 초기화 및 UI 복구
        bIsMeetingChatbotActive = false;
        
        if (InviteBtn && NaNumiSizeBox && MumuLeeSizeBox)
        {
            // 아이콘 UI 상태를 일반 모드로 복구 (무물이는 숨기고 나눔이/초대 버튼 표시)
            // (아래에서 새 방 진입 시 다시 세팅하므로 여기선 생략 가능하지만 안전하게 처리)
            MumuLeeSizeBox->SetVisibility(ESlateVisibility::Collapsed);
            //InviteBtn->SetVisibility(ESlateVisibility::Visible);
            NaNumiSizeBox->SetVisibility(ESlateVisibility::Visible);
        }
        
        // 버튼 색상 초기화
        UpdateQuestionButtonState();
    }

    // =================================================================================
    // 5. UI 교체 (채팅창 내용물 바꾸기)
    // =================================================================================
    RemoveChatBlock(); // 기존 채팅창 제거
    if (SelectedIcon->ChatBlockUI)
    {
        AddChatBlock(SelectedIcon->ChatBlockUI); // 새 채팅창 추가
        SetGroupNameTitle(SelectedIcon->ChatBlockUI->GetTeamName()); // 제목 변경
    }

    // 포인터 교체
    CurrentSelectedGroup = SelectedIcon;

    // 새 아이콘 하이라이트
    if (CurrentSelectedGroup)
    {
        CurrentSelectedGroup->SetHighlight(true);
    }

	if (SelectedIcon)
	{
		SelectedIcon->SetNewMessageNotice(false);
	}

    // =================================================================================
    // 6. [새 방 진입] 방 성격에 따른 로직 분기
    // =================================================================================

    // Case A: '학습 챗봇 방(무물이)' 진입
    if (SelectedIcon->bIsChatbotRoom)
    {
        // 1) 상단 아이콘/버튼 UI 변경 (무물이 ON, 나머지 OFF)
        if (InviteBtn && NaNumiSizeBox && MumuLeeSizeBox)
        {
            if (ChatbotIcon) ChatbotIcon->SetIconIMG(MumuLeeOnIMG);
            MumuLeeSizeBox->SetVisibility(ESlateVisibility::Visible);

            InviteBtn->SetVisibility(ESlateVisibility::Collapsed);
            NaNumiSizeBox->SetVisibility(ESlateVisibility::Collapsed);
        }

        // 2) 채팅창 초기화 (챗봇 방은 들어올 때마다 새로 시작하는 느낌을 위해 클리어할 수도 있고 유지할 수도 있음)
        // 기존 로직 유지: 클리어 후 히스토리 로드
        if (SelectedIcon->ChatBlockUI)
        {
            SelectedIcon->ChatBlockUI->ChatScrollBox->ClearChildren();
        }

        // 3) 대화 내역 불러오기 (HTTP)
        if (GI && HttpSystem)
        {
            HttpSystem->SendChatHistoryRequest(GI->PlayerUniqueID);
        }

        // 4) [수정] 웹소켓 연결 확인 및 학습 세션 시작 (StartLearningChat)
        // 챗봇 방에 들어왔으므로 학습 세션을 시작한다고 서버에 알림
        if (SelectedIcon->ChatBlockUI && SelectedIcon->ChatBlockUI->ChatScrollBox->GetChildrenCount() == 0)
        {
            // 웰컴 메시지 (약간의 딜레이)
            FTimerHandle WelcomeHandle;
            GetWorld()->GetTimerManager().SetTimer(WelcomeHandle, [this]()
            {
                AddBotChat(TEXT("안녕하세요! 무엇을 도와드릴까요?"));
            }, 0.1f, false);
        }

        if (WebSocketSystem)
        {
            // 만약 연결이 안 되어 있다면 연결 시도 (통합 연결)
            if (!WebSocketSystem->IsConnected())
            {
                WebSocketSystem->Connect();
            }

            // 연결이 확실해진 후 Start 패킷 전송
            FTimerHandle ConnectTimerHandle;
            GetWorld()->GetTimerManager().SetTimer(ConnectTimerHandle, [this, UserID]()
            {
                if (!this || !WebSocketSystem) return;

                if (WebSocketSystem->IsConnected())
                {
                    // [핵심 변경] 구조체 전송 함수 대신 래퍼 함수 사용
                    WebSocketSystem->StartLearningChat(UserID, UserID);
                }
            }, 0.5f, false);
        }
    }
    // Case B: '일반 채팅 방' 진입
    else
    {
        // 1) 상단 아이콘/버튼 UI 변경 (무물이 OFF, 나눔이/초대 ON)
        if (InviteBtn && QuestionBtn && NaNumiSizeBox && MumuLeeSizeBox)
        {
            if (ChatbotIcon) ChatbotIcon->SetIconIMG(MumuLeeOffIMG);
            MumuLeeSizeBox->SetVisibility(ESlateVisibility::Collapsed);

            //InviteBtn->SetVisibility(ESlateVisibility::Visible);
            NaNumiSizeBox->SetVisibility(ESlateVisibility::Visible);

            // 방을 옮겼으므로 AI 도우미(나눔이)는 꺼진 상태로 시작
            bIsMeetingChatbotActive = false;
            UpdateQuestionButtonState();
        }

        // 2) HTTP로 지난 대화 내역 불러오기
        if (HttpSystem)
        {
        	if (SelectedIcon->ChatBlockUI)
        	{
        		SelectedIcon->ChatBlockUI->ChatScrollBox->ClearChildren();
        	}
        	
            HttpSystem->SendTeamChatMessageRequest(SelectedIcon->ChatBlockUI->GetTeamID());
        }

        // 3) 보이스 채널 변경 (Vivox 등)
        if (AMumulPlayerState* PS = Cast<AMumulPlayerState>(GetOwningPlayerState()))
        {
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

void UGroupChatUI::OnLearningChatStarted(const FLearningResponsePayload& Info)
{
	// Info.message 에 "새로운 학습 세션이 시작되었습니다." 등이 들어있음
	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
	{
		AddBotChat(Info.message);
	}
}

void UGroupChatUI::OnLearningChatAnswer(const FLearningResponsePayload& Answer)
{
	// Answer.answer 에 답변 내용이 있음
	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
	{
		AddBotChat(Answer.answer);
	}
}

void UGroupChatUI::OnLearningChatEnded(const FLearningResponsePayload& Info)
{
	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
	{
		AddBotChat(Info.message);
	}
}


// [2] 회의 도우미 핸들러
void UGroupChatUI::OnMeetingChatStarted(const FMeetingResponsePayload& Info)
{
	// Info.groupId 확인
	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
	{
		if (CurrentSelectedGroup->ChatBlockUI->GetTeamID() == Info.groupId)
		{
			AddBotChat(FString::Printf(TEXT("[알림] %s"), *Info.message));
		}
	}
}

void UGroupChatUI::OnMeetingAnswer(const FMeetingResponsePayload& Answer)
{
	bool bIsCurrentRoom = (CurrentSelectedGroup && 
							   CurrentSelectedGroup->ChatBlockUI && 
							   CurrentSelectedGroup->ChatBlockUI->GetTeamID() == Answer.groupId);

	if (bIsCurrentRoom)
	{
		AddBotChat(Answer.answer);
	}
	else
	{
		// 안 보고 있는 방이면 알림 표시
		if (UGroupIconUI* TargetIcon = FindGroupIconByTeamID(Answer.groupId))
		{
			TargetIcon->SetNewMessageNotice(true);
		}
	}
}

void UGroupChatUI::OnMeetingChatEnded(const FMeetingResponsePayload& Info)
{
	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
	{
		if (CurrentSelectedGroup->ChatBlockUI->GetTeamID() == Info.groupId)
		{
			AddBotChat(FString::Printf(TEXT("[알림] %s"), *Info.message));
          
			bIsMeetingChatbotActive = false;
			UpdateQuestionButtonState();
		}
	}
}

// // [1] 학습 챗봇 핸들러 (Chatbot_Room 전용)
// void UGroupChatUI::OnLearningChatStarted(FString Message)
// {
// 	// 학습 챗봇 방을 보고 있을 때만 메시지 표시
// 	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
// 	{
// 		AddBotChat(Message);
// 	}
// }
//
// void UGroupChatUI::OnLearningChatAnswer(FString Answer)
// {
// 	// 학습 챗봇은 무조건 챗봇 전용 방에만 뜹니다.
// 	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
// 	{
// 		AddBotChat(Answer);
// 	}
// }
//
// void UGroupChatUI::OnLearningChatEnded(FString Message)
// {
// 	if (CurrentSelectedGroup && CurrentSelectedGroup->bIsChatbotRoom)
// 	{
// 		AddBotChat(Message);
// 	}
// }
//
//
// // [2] 회의 도우미 핸들러 (일반 채팅방 전용)
// void UGroupChatUI::OnMeetingChatStarted(FString Message, FString GroupId, FString UserName)
// {
// 	// 내가 보고 있는 방이, 도우미가 시작된 그 방인지 확인
// 	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
// 	{
// 		if (CurrentSelectedGroup->ChatBlockUI->GetTeamID() == GroupId)
// 		{
// 			// 예: "[알림] 홍길동님이 회의 도우미를 시작했습니다." 같은 시스템 메시지로 띄울 수도 있음
// 			AddBotChat(FString::Printf(TEXT("[알림] %s"), *Message));
// 		}
// 	}
// }
//
// void UGroupChatUI::OnMeetingChatAnswer(FString Answer, FString GroupId)
// {
// 	// 답변이 도착한 방(GroupId)이 현재 보고 있는 방과 일치하는지 확인
// 	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
// 	{
// 		if (CurrentSelectedGroup->ChatBlockUI->GetTeamID() == GroupId)
// 		{
// 			AddBotChat(Answer);
// 		}
// 	}
// }
//
// void UGroupChatUI::OnMeetingChatEnded(FString Message, FString GroupId)
// {
// 	if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
// 	{
// 		if (CurrentSelectedGroup->ChatBlockUI->GetTeamID() == GroupId)
// 		{
// 			AddBotChat(FString::Printf(TEXT("[알림] %s"), *Message));
//
// 			// 만약 내가 켠 사람이라면 버튼 상태도 꺼줌
// 			bIsMeetingChatbotActive = false;
// 			UpdateQuestionButtonState();
// 		}
// 	}
// }

void UGroupChatUI::OnServerTeamChatMessageResponse(bool bSuccess, FString Message)
{
	// 현재 보고 있는 방이 없거나, 챗봇 방(무물이)을 보고 있다면 무시
    if (!CurrentSelectedGroup || CurrentSelectedGroup->bIsChatbotRoom) return;

    if (bSuccess)
    {
        // 1. JSON 파싱
        TArray<FTeamChatMessageResponse> ChatHistory;
        
        if (FJsonObjectConverter::JsonArrayStringToUStruct(Message, &ChatHistory, 0, 0))
        {
            UE_LOG(LogTemp, Log, TEXT("[TeamChat] Loaded %d messages"), ChatHistory.Num());

            // 2. 현재 보여지고 있는 채팅창 초기화 (중복 방지)
            if (CurrentSelectedGroup->ChatBlockUI)
            {
                CurrentSelectedGroup->ChatBlockUI->ChatScrollBox->ClearChildren();
            }

            // 3. 메시지 순회하며 추가
            for (const FTeamChatMessageResponse& Msg : ChatHistory)
            {
            	if (Msg.role == "user")
            	{
            		TArray<FString> Parts;
            		Msg.formattedCreatedAt.ParseIntoArray(Parts, TEXT(" "));
            		FString TimeOnly = Parts[Parts.Num() - 2] + TEXT(" ") + Parts.Last();


            		int32 targetTendency = 0;
            		FString SlotName = TEXT("IslandMapSave");
            		if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
            		{
            			UMapDataSaveGame* LoadInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

            			if (LoadInst->PlayerTendency.Find(Msg.userId))
            			{
            				targetTendency = *LoadInst->PlayerTendency.Find(Msg.userId) - 1;
            			}
            		}
            		
					AddChat(
					    GetCurrentTeamID(),
					    TimeOnly, 
					    Msg.userId, 
					    *Msg.userName, 
					    *Msg.message,
					    targetTendency
					);
            	}
	            else if (Msg.role == "assistant")
	            {
					//UE_LOG(LogTemp, Warning, TEXT("[TeamChat] Assistant message received"));

	            	TArray<FString> Parts;
	            	Msg.formattedCreatedAt.ParseIntoArray(Parts, TEXT(" "));
	            	FString TimeOnly = Parts[Parts.Num() - 2] + TEXT(" ") + Parts.Last();
	            	
	            	TSubclassOf<UBotChatMessageBlockUI> TargetWidgetClass = MeetingBotChatMessageBlockUIClass;
	            	FString BotName = TEXT("나눔이");
	            	if (TargetWidgetClass)
	            	{
	            		UBotChatMessageBlockUI* BotChat = CreateWidget<UBotChatMessageBlockUI>(GetWorld(), TargetWidgetClass);
	            		if (BotChat)
	            		{
	            			CurrentSelectedGroup->ChatBlockUI->ChatScrollBox->AddChild(BotChat);
	            			BotChat->SetContent(TimeOnly, BotName, Msg.message);
	            		}
	            	}
	            }
            	else
            	{
            		UE_LOG(LogTemp, Warning, TEXT("[TeamChat] message not received"));
            	}
            }
            
            // 4. 스크롤 맨 아래로 내리기
            if (CurrentSelectedGroup->ChatBlockUI)
            {
                // 0.01초 뒤 실행 (위젯이 그려진 후 스크롤 이동)
                FTimerHandle ScrollHandle;
                GetWorld()->GetTimerManager().SetTimer(ScrollHandle, [this]()
                {
                    if (CurrentSelectedGroup && CurrentSelectedGroup->ChatBlockUI)
                    {
                        CurrentSelectedGroup->ChatBlockUI->ChatScrollBox->ScrollToEnd();
                    }
                }, 0.01f, false);
            }
        }
        else
        {
             UE_LOG(LogTemp, Error, TEXT("[TeamChat] JSON Parsing Failed"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[TeamChat] Failed to load history: %s"), *Message);
    }
}

void UGroupChatUI::OnServerChatHistoryResponse(bool bSuccess, FString Message)
{
	// 현재 챗봇 방을 보고 있지 않다면 무시
	if (!CurrentSelectedGroup || !CurrentSelectedGroup->bIsChatbotRoom) return;
	//if (!CurrentSelectedGroup) return;

	if (bSuccess)
	{
		FChatHistoryResponse HistoryData;

		// [핵심 로직] 현재 보고 있는 방의 종류에 따라 클래스와 이름 결정
		TSubclassOf<UBotChatMessageBlockUI> TargetWidgetClass = BotChatMessageBlockUIClass;
		FString BotName = TEXT("무물이");

		
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Message, &HistoryData, 0, 0))
		{
			UE_LOG(LogTemp, Log, TEXT("[ChatHistory] Loaded %d messages"), HistoryData.messages.Num());

			UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
			FString MyName = GI ? GI->PlayerName : TEXT("Me");
			int32 MyID = GI ? GI->PlayerUniqueID : 0;

			// 메시지 순회하며 UI 추가
			for (const FChatHistoryMessage& Msg : HistoryData.messages)
			{
				FString ParsedTime = ParseTimeFromISO8601(Msg.created_at);

				if (Msg.role == TEXT("user"))
				{
					// 내 질문 -> 일반 말풍선 (AddChat)
					// (TeamID는 현재 챗봇방 ID 사용)
					
					int32 targetTendency = 0;
					FString SlotName = TEXT("IslandMapSave");
					if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
					{
						UMapDataSaveGame* LoadInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

						if (LoadInst->PlayerTendency.Find(MyID))
						{
							targetTendency = *LoadInst->PlayerTendency.Find(MyID) - 1;
						}
					}
					
					AddChat(CurrentSelectedGroup->ChatBlockUI->GetTeamID(), ParsedTime, MyID, MyName, Msg.content, targetTendency);
				}
				else if (Msg.role == TEXT("assistant"))
				{

					if (TargetWidgetClass)
					{
						UBotChatMessageBlockUI* BotChat = CreateWidget<UBotChatMessageBlockUI>(GetWorld(), TargetWidgetClass);
						if (BotChat)
						{
							CurrentSelectedGroup->ChatBlockUI->ChatScrollBox->AddChild(BotChat);
							BotChat->SetContent(ParsedTime, BotName, Msg.content);
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

	// 현재 선택된 그룹이 없으면 중단 (방어 코드)
	if (!CurrentSelectedGroup) return;

	// [핵심 로직] 현재 보고 있는 방의 종류에 따라 클래스와 이름 결정
	TSubclassOf<UBotChatMessageBlockUI> TargetWidgetClass = nullptr;
	FString BotName = TEXT("");

	// 1. 학습 챗봇 방인 경우 ("무물이")
	if (CurrentSelectedGroup->bIsChatbotRoom)
	{
		TargetWidgetClass = BotChatMessageBlockUIClass;
		BotName = TEXT("무물이");
	}
	// 2. 일반 그룹 채팅방인 경우 ("나눔이")
	else
	{
		TargetWidgetClass = MeetingBotChatMessageBlockUIClass;
		BotName = TEXT("나눔이");
	}

	// 위젯 생성 및 추가
	if (TargetWidgetClass)
	{
		UBotChatMessageBlockUI* BotChat = CreateWidget<UBotChatMessageBlockUI>(GetWorld(), TargetWidgetClass);
		if (BotChat)
		{
			ChatChunk->ChatScrollBox->AddChild(BotChat);

			FString TimeStamp = MakeChatTimeStamp();

			// 내용 설정
			BotChat->SetContent(TimeStamp, BotName, Message);

			// 스크롤 내리기
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, [ChatChunk]()
			{
				if (IsValid(ChatChunk) && IsValid(ChatChunk->ChatScrollBox))
				{
					ChatChunk->ChatScrollBox->ScrollToEnd();
					//ChatChunk->ChatScrollBox->SetScrollOffset(ChatChunk->ChatScrollBox->);
				}
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

FReply UGroupChatUI::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 입력창이 없거나, 입력창에 포커스가 아니면 건드리지 않음
	if (!EditBox || !EditBox->HasKeyboardFocus())
	{
		return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
	}

	const FKey Key = InKeyEvent.GetKey();
	const bool bShiftDown = InKeyEvent.IsShiftDown();

	// Enter 처리
	if (Key == EKeys::Enter)
	{
		// Shift+Enter => 줄바꿈 허용 (기존 동작)
		if (bShiftDown)
		{
			return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent); // Unhandled 쪽으로 흐르게
		}

		// Enter => 줄바꿈 막고 전송
		OnTextBoxCommitted();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UGroupChatUI::OnTextBoxCommitted()
{
    FText Text = EditBox->GetText();
    if (Text.IsEmpty()) return;
    if (!CurrentSelectedGroup) return;

    FString Content = Text.ToString();
    FString TimeStamp = MakeChatTimeStamp();

    UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
    FString MyName = GI ? GI->PlayerName : TEXT("Me");
    int32 MyID = GI ? GI->PlayerUniqueID : 0;

    FString Grade = TEXT("초급");
    switch (Difficulty)
    {
    case EMumuLeeDifficulty::Beginner:  Grade = TEXT("초급"); break;
    case EMumuLeeDifficulty::Normal:    Grade = TEXT("중급"); break;
    case EMumuLeeDifficulty::Advanced:  Grade = TEXT("고급"); break;
    }

    // [전송 로직 분기]
    if (CurrentSelectedGroup->bIsChatbotRoom)
    {
       AddChat(CurrentSelectedGroup->ChatBlockUI->GetTeamID(), TimeStamp, MyID, MyName, Content, 0);
       
       // === Case A: 학습 챗봇 방 ===
       if (WebSocketSystem && WebSocketSystem->IsConnected())
       {
          // [신규 함수 사용]
          WebSocketSystem->QueryLearningChat(MyID, MyID, Content, Grade);
       }
       else
       {
          AddBotChat(TEXT("무물이와 연결되어 있지 않습니다."));
          WebSocketSystem->Connect(); // 재연결 시도
       }
    }
    else
    {
       // === Case B: 일반 그룹 채팅방 ===
        if (CurrentSelectedGroup->ChatBlockUI)
        {
          	FString TeamID = CurrentSelectedGroup->ChatBlockUI->GetTeamID();
       	  	bool bSentToBot = false;
		  	
          	// 1. [DB 저장] HTTP 전송 (기존 유지)
          	// if (HttpSystem)
          	// {
          	//    FString Now = FDateTime::Now().ToString(TEXT("%H:%M"));
          	//    HttpSystem->SendChatMessageRequest(TeamID, MyID, Content, Now);
          	// }
		  	
          	// 2. [채팅 공유] RPC 전송 (기존 유지)

		  	
          	// 3. [AI 질문] 도우미 (웹소켓)
          	if (bIsMeetingChatbotActive)
          	{
          	   	if (WebSocketSystem && WebSocketSystem->IsConnected())
          	   	{
          	   	   // [신규 함수 사용]
          	   	   	WebSocketSystem->QueryMeetingChat(TeamID, MyID, MyName, Content);
          	   	   	UE_LOG(LogTemp, Log, TEXT("[MeetingBot] Query Sent: %s"), *Content);
          	   		bSentToBot = true;
          	   	}
          	   	else
          	   	{
          	   	   	AddBotChat(TEXT("회의 도우미와 연결이 끊겼습니다."));
          	   	   	bIsMeetingChatbotActive = false;
          	   	   	UpdateQuestionButtonState();
          	   	   	WebSocketSystem->Connect(); // 재연결 시도
          	   	}
          	}

        	if (!bSentToBot && HttpSystem) 
        	{
        		FString Now = FDateTime::Now().ToString(TEXT("%H:%M"));
        		HttpSystem->SendChatMessageRequest(TeamID, MyID, Content, Now);
        	}

        	TArray<int32> UserIDs;
        	CurrentSelectedGroup->ChatBlockUI->GetTeamUsers().GetKeys(UserIDs);
        	if (ACuteAlienController* PC = Cast<ACuteAlienController>(GetOwningPlayer()))
        	{
        		if (AMumulPlayerState* PS = Cast<AMumulPlayerState>(PC->PlayerState))
        		PC->ChatComp->Server_RequestChat(TeamID, UserIDs, TimeStamp, MyID, MyName, Content, PS->PS_TendencyID);
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

FString UGroupChatUI::MakeChatTimeStamp()
{
	const FDateTime Now = FDateTime::Now();
	const int32 H = Now.GetHour();
	const int32 Hour12 = (H % 12 == 0) ? 12 : H % 12;

	return FString::Printf(
		TEXT("%s %02d:%02d"),
		H >= 12 ? TEXT("PM") : TEXT("AM"),
		Hour12,
		Now.GetMinute()
	);
}

void UGroupChatUI::AddChat(const FString& TeamID, const FString& CurrentTime, const int32& UserID, const FString& Name,
                           const FString& Text, const int32& TendencyID) const
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
		Chat->SetProfileIMG(IMGManager->GetImageByUserID(TendencyID));

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
	else
	{
		
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
					PS->ChatComp->Server_AddTeamChatList(TeamChat.teamChatId);
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
	LinkedPlayerUI->TryLockUI(0.5f);
	
	if (IsAnimationPlaying(CreateGroupUI_Slide)) return;

	CreateGroupChatUI->RefreshJoinedPlayerList();

	if (bCreateGroupVisible)
	{
		bCreateGroupVisible = false;
		PlayAnimation(CreateGroupUI_Slide, 0, 1, EUMGSequencePlayMode::Reverse);
		AudioManager->PlayPopDownSound();
	}
	else
	{
		bCreateGroupVisible = true;

		if (!bIsToggled)
		{
			ToggleGroupChatAlignment();
		}

		if (LinkedPlayerUI)
		{
			LinkedPlayerUI->CloseCustomUI();
			LinkedPlayerUI->CloseHousingUI();
			LinkedPlayerUI->CloseNoticeUI();
          
			// PlayerUI가 채팅창이 열렸음을 알도록 플래그 설정 (friend class가 아니면 함수 필요)
			// 여기서는 PlayerUI쪽에서 bIsOpenChatUI를 public으로 풀거나 Setter가 있다고 가정
			// 혹은 ToggleGroupChatAlignment 내부에서 처리됨
		}

		PlayAnimation(CreateGroupUI_Slide);
		AudioManager->PlayPopUpSound();
	}

	// [중요] 상태 변경 후 마우스 모드 갱신
	if (LinkedPlayerUI)
	{
		LinkedPlayerUI->RefreshInputMode();
	}
}

void UGroupChatUI::AddGroupIcon(UGroupIconUI* UI) const
{
	GroupScrollBox->AddChild(UI);
}

void UGroupChatUI::ToggleInvitationUI()
{
	InvitationUI->RefreshJoinedPlayerList();

	ToggleVisibility(InvitationBox);
	if (CreateGroupChatBox->GetVisibility() == ESlateVisibility::Visible)
	{
		ToggleVisibility(CreateGroupChatBox);
	}
}

void UGroupChatUI::ToggleGroupChatAlignment()
{
	LinkedPlayerUI->TryLockUI(0.5f);
	
	bIsToggled = !bIsToggled;

	if (LinkedPlayerUI)
	{
		if (bIsToggled)
		{
			LinkedPlayerUI->CloseCustomUI();
			LinkedPlayerUI->CloseHousingUI();
			LinkedPlayerUI->CloseNoticeUI();
          
			LinkedPlayerUI->bIsOpenChatUI = true; 
		}
		else
		{
			LinkedPlayerUI->bIsOpenChatUI = false;
		}
	}

	if (!bIsToggled && bCreateGroupVisible)
	{
		bCreateGroupVisible = false;
		PlayAnimation(CreateGroupUI_Slide, 0, 1, EUMGSequencePlayMode::Reverse);
	}
    
	// 버튼 이미지 처리
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D(30.f, 50.f);
	Brush.SetResourceObject(bIsToggled ? RightIMG : LeftIMG);
	ArrowBtnImg->SetBrush(Brush);

	// 슬라이드 애니메이션 시작
	StartVal = AlignmentVal;
	TargetVal = bIsToggled ? 0.76f : 0.f;
	Elapsed = 0.f;
	bAnimating = true;

	if (LinkedPlayerUI)
	{
		LinkedPlayerUI->RefreshInputMode();
	}
}

void UGroupChatUI::InitPlayerUI(UPlayerUI* InPlayerUI)
{
	LinkedPlayerUI = InPlayerUI;
}

void UGroupChatUI::CloseChatUIPannel()
{
	if (bCreateGroupVisible)
	{
		bCreateGroupVisible = false;
		PlayAnimation(CreateGroupUI_Slide, 0, 1, EUMGSequencePlayMode::Reverse);
	}

	if (bIsToggled)
	{
		ToggleGroupChatAlignment(); 
	}
}

void UGroupChatUI::OnToggleVisibilityBtn()
{
	ToggleGroupChatAlignment();
}

void UGroupChatUI::OnAICheckStateChanged(bool bIsChecked)
{
	// 일반 방이 아니면 무시
	if (!CurrentSelectedGroup || CurrentSelectedGroup->bIsChatbotRoom) return;
    
	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	if (!GI) return;

	int32 MyID = GI->PlayerUniqueID;
	FString MyName = GI->PlayerName;
	FString GroupID = GetCurrentTeamID();

	bIsMeetingChatbotActive = bIsChecked;
	UpdateQuestionButtonState();

	if (bIsChecked)
	{
		// [ON] StartMeetingChat 호출
		if (WebSocketSystem)
		{
			if (!WebSocketSystem->IsConnected()) WebSocketSystem->Connect();

			// 약간의 딜레이 후 시작 (연결 보장)
			FTimerHandle Handle;
			GetWorld()->GetTimerManager().SetTimer(Handle, [this, MyID, MyName, GroupID]()
			{
			   if (WebSocketSystem && WebSocketSystem->IsConnected())
			   {
				  // [신규 함수 사용]
				  WebSocketSystem->StartMeetingChat(GroupID, MyID, MyName);
				  UE_LOG(LogTemp, Log, TEXT("[MeetingBot] Started for Group: %s"), *GroupID);
			   }
			}, 0.2f, false);
		}
	}
	else
	{
		// [OFF] EndMeetingChat 호출
		if (WebSocketSystem && WebSocketSystem->IsConnected())
		{
			// [신규 함수 사용]
			WebSocketSystem->EndMeetingChat(GroupID);
			UE_LOG(LogTemp, Log, TEXT("[MeetingBot] Ended"));
		}
	}
}

void UGroupChatUI::OnClickQuestionBtn()
{
	if (ACuteAlienController* PC = Cast<ACuteAlienController>(GetOwningPlayer()))
	{
		if (PC->MeetingComp && !PC->MeetingComp->CurrentMeetingSessionID.IsEmpty())
		{
			AddBotChat(TEXT("회의 중에는 나눔이를 사용할 수 없습니다."));
			return; // 함수 종료
		}
	}

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

void UGroupChatUI::OnReceiveRealtimeChat(const FString& TeamID, const FString& CurrentTime, int32 UserID,
	const FString& UserName, const FString& Message, int32 TendencyID)
{
	bool bIsCurrentRoom = (CurrentSelectedGroup && 
						   CurrentSelectedGroup->ChatBlockUI && 
						   CurrentSelectedGroup->ChatBlockUI->GetTeamID() == TeamID);

	if (bIsCurrentRoom)
	{
		// [A] 보고 있는 방 -> 채팅창에 바로 추가
		// (RPC로 넘어온 CurrentTime을 그대로 사용)
		AddChat(TeamID, CurrentTime, UserID, UserName, Message, TendencyID);
	}
	else
	{
		// [B] 안 보고 있는 방 -> 해당 아이콘 찾아서 '알림(NewMessageNotice)' 켜기
		if (UGroupIconUI* TargetIcon = FindGroupIconByTeamID(TeamID))
		{
			TargetIcon->SetNewMessageNotice(true);
		}
	}
}

UGroupIconUI* UGroupChatUI::FindGroupIconByTeamID(const FString& TeamID)
{
	if (!GroupScrollBox) return nullptr;

	for (UWidget* Child : GroupScrollBox->GetAllChildren())
	{
		if (UGroupIconUI* IconUI = Cast<UGroupIconUI>(Child))
		{
			if (IconUI->ChatBlockUI && IconUI->ChatBlockUI->GetTeamID() == TeamID)
			{
				return IconUI;
			}
		}
	}
	return nullptr;
}

void UGroupChatUI::OnUpdateMeetingStatus(const FString& TeamID, bool bIsActive)
{
	UGroupIconUI* TargetIcon = FindGroupIconByTeamID(TeamID);
	if (TargetIcon)
	{
		TargetIcon->SetMeetingStatus(bIsActive);
	}

	if (GetCurrentTeamID() == TeamID)
	{
		bIsMeetingChatbotActive = bIsActive;
		UpdateQuestionButtonState(); 
		OnRecordBtnState(bIsActive); 
	}
}

void UGroupChatUI::UpdateDot()
{
	if (!IsValid(this))
		return;

	if (!RecordText1 || !RecordText1->BaseText)
		return;

	DotCount = (DotCount % 3) + 1;

	FString Dots = FString::ChrN(DotCount, TEXT('.'));
	FString Text = FString::Printf(TEXT("기록중%s"), *Dots);

	RecordText1->BaseText->SetText(FText::FromString(Text));
}


void UGroupChatUI::OnRecordBtnState(bool bIsOn)
{
	if (!IsValid(RecordText0) || !IsValid(RecordText1))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Text"));
		return;
	}

	if (!IsValid(RecordText0->BaseText) || !IsValid(RecordText1->BaseText))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid BaseText"));
		return;
	}

	// ----------------------------------------------------
	// 0) 공통 방어
	// ----------------------------------------------------
	if (!RecordIMG || RecordIMGs.Num() < 2 || !RecordText0 || !RecordText1)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRecordBtnState: Null UI element detected."));
		return;
	}

	if (!RecordText0->BaseText || !RecordText1->BaseText)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRecordBtnState: BaseText is NULL."));
		return;
	}


	// ----------------------------------------------------
	// 1) ON 상태
	// ----------------------------------------------------
	if (bIsOn)
	{
		if (RecordIMGs.IsValidIndex(1))
		{
			RecordIMG->SetBrushFromTexture(RecordIMGs[1]);
		}

		RecordText0->BaseText->SetText(FText::FromString(TEXT("나눔이가")));
		// 혹시 Text1이 ON에도 쓰고 싶으면 여기 넣으면 됨

		// 타이머 방어: 기존 타이머 제거 후 다시 시작
		GetWorld()->GetTimerManager().ClearTimer(DotTimer);

		GetWorld()->GetTimerManager().SetTimer(
			DotTimer,
			this,
			&UGroupChatUI::UpdateDot,
			1.1f,
			true
		);
	}

	// ----------------------------------------------------
	// 2) OFF 상태
	// ----------------------------------------------------
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(DotTimer);

		if (RecordIMGs.IsValidIndex(0))
		{
			RecordIMG->SetBrushFromTexture(RecordIMGs[0]);
		}

		RecordText0->BaseText->SetText(FText::FromString(TEXT("나눔이로")));
		RecordText1->BaseText->SetText(FText::FromString(TEXT("기록하기")));
	}
}

void UGroupChatUI::OnBeginnerClicked()
{
	Difficulty = EMumuLeeDifficulty::Beginner;
	BeginnerBtn->SetBackgroundColor(FLinearColor(0.2f, 1.0f, 0.2f, 1.0f));
	IntermediateBtn->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	AdvancedBtn->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
}

void UGroupChatUI::OnNormalClicked()
{
	Difficulty = EMumuLeeDifficulty::Normal;
	IntermediateBtn->SetBackgroundColor(FLinearColor(0.2f, 1.0f, 0.2f, 1.0f));
	BeginnerBtn->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	AdvancedBtn->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
}

void UGroupChatUI::OnAdvancedClicked()
{
	Difficulty = EMumuLeeDifficulty::Advanced;
	AdvancedBtn->SetBackgroundColor(FLinearColor(0.2f, 1.0f, 0.2f, 1.0f));
	BeginnerBtn->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	IntermediateBtn->SetBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
}
