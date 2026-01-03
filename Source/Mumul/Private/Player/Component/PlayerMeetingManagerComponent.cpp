// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/PlayerMeetingManagerComponent.h"

#include "Base/MumulGameInstance.h"
#include "Base/MumulGameState.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Data/AudioManager.h"
#include "GameFramework/GameStateBase.h"
#include "Net/VoiceConfig.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "Player/MumulPlayerState.h"
#include "Player/VoiceChatComponent.h"
#include "Player/Component/PlayerChatComponent.h"
#include "UI/GroupChatUI.h"
#include "UI/VoiceMeetingUI.h"
#include "UI/BaseUI/BaseText.h"


class UMumulGameInstance;
// Sets default values for this component's properties
UPlayerMeetingManagerComponent::UPlayerMeetingManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	static ConstructorHelpers::FObjectFinder<USoundAttenuation> SilentAttFinder(
		TEXT("/Game/Khc/Audio/SA_Silent.SA_Silent")); // 예시 경로
	if (SilentAttFinder.Succeeded())
	{
		SilentAttenuation = SilentAttFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundAttenuation> NormalAttFinder(
		TEXT("/Game/Khc/Audio/SA_Proximity.SA_Proximity")); // 경로 확인 필수!
	if (NormalAttFinder.Succeeded())
	{
		NormalAttenuation = NormalAttFinder.Object;
	}

	static ConstructorHelpers::FClassFinder<UVoiceMeetingUI> WidgetFinder(
		TEXT("/Game/Khc/Blueprint/UI/WBP_CreateMeeting.WBP_CreateMeeting_C")); // 경로 확인 필수!
	if (WidgetFinder.Succeeded())
	{
		VoiceMeetingUIClass = WidgetFinder.Class;
	}
}


// Called when the game starts
void UPlayerMeetingManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	owner = Cast<ACuteAlienController>(GetOwner());
	
	if (owner)
		player = Cast<ACuteAlienPlayer>(owner->GetPawn()); 

	
	if (owner && owner->IsLocalController())
	{
		UMumulGameInstance* GI = Cast<UMumulGameInstance>(owner->GetGameInstance());
		if (GI)
		{
			// 여기서 로컬 변수로 받아와서 바인딩 (Controller의 변수에 의존 X)
			UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>();
			if (HttpSystem)
			{
				HttpSystem->OnStartMeeting.AddDynamic(this, &UPlayerMeetingManagerComponent::OnStartMeetingResponse);
				HttpSystem->OnJoinMeeting.AddDynamic(this, &UPlayerMeetingManagerComponent::OnJoinMeetingResponse);
			}
		}
	
		if (VoiceMeetingUIClass)
		{
			VoiceMeetingUI = CreateWidget<UVoiceMeetingUI>(owner, VoiceMeetingUIClass);
			if (VoiceMeetingUI)
			{
				VoiceMeetingUI->AddToViewport(7);
				VoiceMeetingUI->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}


// Called every frame
void UPlayerMeetingManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerMeetingManagerComponent::RequestStartMeetingRecording(FString InMeetingTitle, FString InAgenda,
	FString InDesc)
{
	AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>();
	UMumulGameInstance* GI = Cast<UMumulGameInstance>(owner->GetGameInstance());

	if (PS && GI)
	{
		FString ChannelID = PS->VoiceChannelID;

		// [HTTP] 방장(Organizer)이 Start Meeting API 호출
		// (서버 응답이 오면 OnStartMeetingResponse가 실행됨)
		if (owner->HttpSystem)
		{
			owner->HttpSystem->StartMeetingRequest(
				InMeetingTitle,
				PS->VoiceChannelID,
				GI->PlayerUniqueID, // Organizer ID
				InAgenda,
				InDesc
			);

			UE_LOG(LogTemp, Log, TEXT("[Meeting] Requesting Start Meeting API..."));
		}
	}
}

void UPlayerMeetingManagerComponent::RequestStopMeetingRecording()
{
	AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>();
	UMumulGameInstance* GI = Cast<UMumulGameInstance>(owner->GetGameInstance());

	if (PS && GI)
	{
		Server_StopChannelRecording(PS->VoiceChannelID);
	}
}

void UPlayerMeetingManagerComponent::Server_StartChannelRecording_Implementation(const FString& TargetChannelID)
{
	UE_LOG(LogTemp, Warning, TEXT("[Server] Request Start for Ch: %s"), *TargetChannelID);


	// [핵심 변경] GameState를 통해 접속한 모든 플레이어(Controller)를 찾음
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			for (APlayerState* PS : GameState->PlayerArray)
			{
				if (!PS) continue;

				AMumulPlayerState* MumulPS = Cast<AMumulPlayerState>(PS);
				// 채널이 같은지 확인
				if (MumulPS && MumulPS->VoiceChannelID == TargetChannelID)
				{
					// 해당 플레이어의 컨트롤러를 가져옴 (서버에는 모든 컨트롤러가 있음)
					if (ACuteAlienController* TargetPC = Cast<ACuteAlienController>(PS->GetOwner()))
					{
						// 그 컨트롤러에게 "녹음 시작해"라고 명령 (Client RPC)
						TargetPC->MeetingComp->Client_StartChannelRecording(TargetChannelID);

						UE_LOG(LogTemp, Log, TEXT("[Server] Sent Start Command to: %s"), *PS->GetPlayerName());
					}
				}
			}
		}
	}
}

void UPlayerMeetingManagerComponent::Client_StartChannelRecording_Implementation(const FString& TargetChannelID)
{
	if (player)
	{
		if (UVoiceChatComponent* VoiceComp = player->FindComponentByClass<UVoiceChatComponent>())
		{
			VoiceComp->StartRecording(); // 실제 녹음 시작

			UE_LOG(LogTemp, Warning, TEXT(">>> [RECORD START] MeetingID: %s"), *TargetChannelID);
		}
	}
}

void UPlayerMeetingManagerComponent::Server_StopChannelRecording_Implementation(const FString& TargetChannelID)
{
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			for (APlayerState* PS : GameState->PlayerArray)
			{
				if (!PS) continue;

				AMumulPlayerState* MumulPS = Cast<AMumulPlayerState>(PS);
				if (MumulPS && MumulPS->VoiceChannelID == TargetChannelID)
				{
					if (ACuteAlienController* TargetPC = Cast<ACuteAlienController>(PS->GetOwner()))
					{
						// 그 컨트롤러에게 "녹음 꺼"라고 명령 (Client RPC)
						TargetPC->MeetingComp->Client_StopChannelRecording();
					}
				}
			}
		}
	}
}

void UPlayerMeetingManagerComponent::Client_StopChannelRecording_Implementation()
{
	if (player)
	{
		if (UVoiceChatComponent* VoiceComp = player->FindComponentByClass<UVoiceChatComponent>())
		{
			// 1. [순서 변경] 상태 업데이트를 최우선으로 처리
			// 일어서서 채널이 바뀌기 전에, "회의 끝났다"는 정보를 확실히 전파해야 함
			if (owner->HasAuthority())
			{
				VoiceComp->OnRecordingStopped.RemoveDynamic(this, &UPlayerMeetingManagerComponent::OnHostRecordingStopped);
				VoiceComp->OnRecordingStopped.AddDynamic(this, &UPlayerMeetingManagerComponent::OnHostRecordingStopped);

				AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>();
				if (PS)
				{
					// 여기서 모든 클라이언트(나 포함)의 UI를 끕니다.
					// 아직 채널이 바뀌기 전이므로 모든 팀원이 메시지를 잘 받습니다.
					Server_UpdateMeetingStatus(PS->VoiceChannelID, false);
				}
             
				UE_LOG(LogTemp, Warning, TEXT("[Host] Binded OnHostRecordingStopped delegate."));
			}

			// 2. 내 로컬 UI 즉시 끄기 (방어 코드)
			// 혹시 Server_UpdateMeetingStatus가 늦게 와도 즉시 꺼지도록 함
			if (owner->ChatComp && owner->ChatComp->GroupChatUI)
			{
				owner->ChatComp->GroupChatUI->OnRecordBtnState(false);
				owner->ChatComp->GroupChatUI->AddBotChat(TEXT("회의가 종료되었습니다."));
             
				// [추가] 중요! 아이콘 상태도 즉시 끔 (MeetingActiveNotice 관련)
				owner->ChatComp->GroupChatUI->OnUpdateMeetingStatus(owner->ChatComp->GroupChatUI->GetCurrentTeamID(), false);
			}

			// 3. 일어서기 (이제 채널이 바뀌어도 상관없음)
			if (owner)
			{
				owner->SetIgnoreMoveInput(false);
				owner->SetIgnoreLookInput(false);
				owner->Server_StandUpFromMeeting();
			}
          
			// 4. 녹음 중지
			VoiceComp->StopRecording();

			if (!owner->HasAuthority())
			{
				CurrentMeetingSessionID = TEXT("");
			}

			UE_LOG(LogTemp, Warning, TEXT(">>> [RECORD STOP]"));
		}
	}
}

void UPlayerMeetingManagerComponent::Server_BroadcastJoinMeeting_Implementation(const FString& TargetChannelID,
	const FString& MeetingID)
{
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			for (APlayerState* PS : GameState->PlayerArray)
			{
				if (PS->GetOwner() == owner)
				{
					continue; // 나는 패스
				}

				AMumulPlayerState* MumulPS = Cast<AMumulPlayerState>(PS);

				// 같은 채널에 있는 다른 사람들에게만 전송
				if (MumulPS && MumulPS->VoiceChannelID == TargetChannelID)
				{
					if (ACuteAlienController* TargetPC = Cast<ACuteAlienController>(PS->GetOwner()))
					{
						TargetPC->MeetingComp->Client_RequestJoinMeeting(MeetingID);
					}
				}
			}
		}
	}
}

void UPlayerMeetingManagerComponent::Client_RequestJoinMeeting_Implementation(const FString& MeetingID)
{
	CurrentMeetingSessionID = MeetingID;

	UMumulGameInstance* GI = Cast<UMumulGameInstance>(owner->GetGameInstance());
	if (GI)
	{
		if (owner->HttpSystem)
		{
			// [HTTP] Join Meeting API 호출
			owner->ChatComp->GroupChatUI->OnRecordBtnState(true);
			owner->HttpSystem->JoinMeetingRequest(GI->PlayerUniqueID, MeetingID);
		}
	}
}

void UPlayerMeetingManagerComponent::OnHostRecordingStopped()
{
	if (player)
	{
		if (UVoiceChatComponent* VoiceComp = player->FindComponentByClass<UVoiceChatComponent>())
		{
			VoiceComp->OnRecordingStopped.RemoveDynamic(this, &UPlayerMeetingManagerComponent::OnHostRecordingStopped);
		}
	}

	// [안전장치] 이미 처리되었으면 무시
	if (CurrentMeetingSessionID.IsEmpty())
	{
		return;
	}

	// 2. [수정] 3초 딜레이 후 회의 종료 요청 (오디오 업로드 대기)
	// 네트워크 속도에 따라 시간을 조절하세요 (2.0f ~ 5.0f)
	float UploadWaitTime = 0.3f;

	FTimerHandle WaitTimerHandle;

	AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>();
	if (PS)
	{
		Server_UnregisterMeetingState(PS->VoiceChannelID);
	}

	owner->GetWorldTimerManager().SetTimer(WaitTimerHandle, [this]()
	{
		// 람다 실행 시점에 컨트롤러가 살아있는지 확인
		if (!IsValid(this)) return;

		// ID가 그새 비워졌는지 다시 확인
		if (CurrentMeetingSessionID.IsEmpty()) return;

		UMumulGameInstance* GI = Cast<UMumulGameInstance>(owner->GetGameInstance());
		if (GI)
		{
			if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
			{
				// [수정] 타이머 없이 즉시 호출!
				// (VoiceComponent가 이미 업로드 완료를 보장하고 호출했기 때문)
				UE_LOG(LogTemp, Warning, TEXT("[HTTP] Requesting End Meeting API... (Upload Confirmed)"));
				HttpSystem->EndMeetingRequest(CurrentMeetingSessionID);

				CurrentMeetingSessionID = TEXT("");
			}
		}
	}, UploadWaitTime, false);
}

void UPlayerMeetingManagerComponent::Server_UpdateMeetingStatus_Implementation(const FString& TeamID, bool bMeetingActive)
{
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			for (APlayerState* PS : GameState->PlayerArray)
			{
				AMumulPlayerState* MumulPS = Cast<AMumulPlayerState>(PS);
                
				if (MumulPS && MumulPS->VoiceChannelID == TeamID)
				{
					for (auto dt : MumulPS->PS_PlayerTeamList)
					{
						if (dt.UniqueTeamID == TeamID)
						{
							if (ACuteAlienController* TargetPC = Cast<ACuteAlienController>(PS->GetOwner()))
							{
								if (TargetPC->ChatComp)
								{
									TargetPC->ChatComp->Client_UpdateMeetingStatus(TeamID, bMeetingActive);
								}
							}
						}
					}

				}
			}
		}
	}
}

void UPlayerMeetingManagerComponent::OpenMeetingSetupUI()
{
	if (VoiceMeetingUIClass && owner)
	{
		if (!VoiceMeetingUI)
		{
			VoiceMeetingUI = CreateWidget<UVoiceMeetingUI>(owner, VoiceMeetingUIClass);
			VoiceMeetingUI->AddToViewport(7);
		}
		owner->SetIgnoreMoveInput(true);
		
		VoiceMeetingUI->ResetInputFields();

		VoiceMeetingUI->InitMeetingUI(true); // 방장 모드
		VoiceMeetingUI->TeamNameText->BaseText->SetText(FText::FromString(owner->ChatComp->GroupChatUI->GetCurrentTeamName()));
		VoiceMeetingUI->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		owner->AudioManager->PlayPopUpSound();
		VoiceMeetingUI->PlayAnimation(VoiceMeetingUI->StartMeeting_SlideAnim);
	}
}

void UPlayerMeetingManagerComponent::OpenEndMeetingPopup()
{
	if (VoiceMeetingUIClass)
	{
		if (!VoiceMeetingUI)
		{
			VoiceMeetingUI = CreateWidget<UVoiceMeetingUI>(owner, VoiceMeetingUIClass);
			VoiceMeetingUI->AddToViewport(7);
		}

		// 종료 확인 화면(Index 1)으로 전환
		if (VoiceMeetingUI->MeetingWidgetSwitcher)
		{
			VoiceMeetingUI->MeetingWidgetSwitcher->SetActiveWidgetIndex(1);
		}

		VoiceMeetingUI->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		owner->AudioManager->PlayPopUpSound();
		VoiceMeetingUI->PlayAnimation(VoiceMeetingUI->EndMeeting_SlideAnim);
	}
}

void UPlayerMeetingManagerComponent::UpdateVoiceChannelMuting()
{
	if (!owner)
	{
		owner = Cast<ACuteAlienController>(GetOwner());
	}

	// [수정 2] 그래도 없으면 리턴 (방어 코드)
	if (!owner) 
	{
		// 아직 오너가 준비되지 않았으므로 무시 (나중에 다시 호출되거나 BeginPlay 이후 정상 작동)
		return; 
	}
	
	AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>();
	if (!PS) return;

	FString MyChannelID = PS->VoiceChannelID;

	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			for (APlayerState* OtherPS : GameState->PlayerArray)
			{
				if (OtherPS == PS) continue;

				AMumulPlayerState* AlienOtherPS = Cast<AMumulPlayerState>(OtherPS);
				if (!AlienOtherPS) continue;

				// [1. Talker 찾기 및 생성 (GC 방지 포함)]
				UVOIPTalker* Talker = nullptr;
				if (CachedTalkers.Contains(AlienOtherPS->PS_UserIndex))
				{
					Talker = CachedTalkers[AlienOtherPS->PS_UserIndex];
				}

				if (!Talker)
				{
					Talker = UVOIPStatics::GetVOIPTalkerForPlayer(OtherPS->GetUniqueId());
				}

				if (!Talker)
				{
					Talker = UVOIPTalker::CreateTalkerForPlayer(OtherPS);
					if (Talker)
					{
						CachedTalkers.Add(AlienOtherPS->PS_UserIndex, Talker);
						UE_LOG(LogTemp, Warning, TEXT("[Voice] Created & Cached Talker for %s"),
						       *OtherPS->GetPlayerName());
					}
				}

				if (Talker)
				{
					// 채널이 같은지 확인
					if (AlienOtherPS->VoiceChannelID == MyChannelID)
					{
						// [Case A] 로비 채널 (3D 거리 기반)
						if (MyChannelID == TEXT("Lobby"))
						{
							APawn* OtherPawn = OtherPS->GetPawn();
							USceneComponent* TargetComponent = OtherPawn ? OtherPawn->GetRootComponent() : nullptr;

							// [핵심] 현재 설정이 목표와 다를 때만 변경 (중복 설정 방지)
							if (Talker->Settings.AttenuationSettings != NormalAttenuation ||
								Talker->Settings.ComponentToAttachTo != TargetComponent)
							{
								Talker->Settings.AttenuationSettings = NormalAttenuation;
								Talker->Settings.ComponentToAttachTo = TargetComponent; // Pawn이 없으면 nullptr이 들어감 (안전함)

								if (TargetComponent)
								{
									UE_LOG(LogTemp, Log, TEXT("[Voice] %s -> Set 3D Lobby Mode (Attached)"),
									       *OtherPS->GetPlayerName());
								}
								else
								{
									UE_LOG(LogTemp, Warning, TEXT("[Voice] %s -> Set 3D Lobby Mode (Pending Pawn...)"),
									       *OtherPS->GetPlayerName());
								}
							}
						}
						// [Case B] 일반 그룹/회의 채널 (2D 전체)
						else
						{
							// 목표: Attenuation 없음(nullptr), 위치 부착 없음(nullptr)
							if (Talker->Settings.AttenuationSettings != nullptr ||
								Talker->Settings.ComponentToAttachTo != nullptr)
							{
								Talker->Settings.AttenuationSettings = nullptr;
								Talker->Settings.ComponentToAttachTo = nullptr;

								UE_LOG(LogTemp, Log, TEXT("[Voice] %s -> Set 2D Team Mode"), *OtherPS->GetPlayerName());
							}
						}
					}
					// [Case C] 다른 채널 (음소거)
					else
					{
						// 목표: Attenuation은 SilentAttenuation
						if (Talker->Settings.AttenuationSettings != SilentAttenuation)
						{
							Talker->Settings.AttenuationSettings = SilentAttenuation;
							Talker->Settings.ComponentToAttachTo = nullptr;

							UE_LOG(LogTemp, Log, TEXT("[Voice] %s -> Muted"), *OtherPS->GetPlayerName());
						}
					}
				}
			}
		}
	}
}

void UPlayerMeetingManagerComponent::OnStartMeetingResponse(bool bSuccess, FString MeetingID)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("[Meeting] Created Successfully: %s"), *MeetingID);

		if (VoiceMeetingUI)
		{
			VoiceMeetingUI->SetMeetingState(true);
		}

		// 1. 미팅 ID 저장
		CurrentMeetingSessionID = MeetingID;

		// 2. [수정] 방장은 Join API 호출 없이 "즉시 녹음 시작"
		if (player)
		{
			if (UVoiceChatComponent* VoiceComp = player->FindComponentByClass<UVoiceChatComponent>())
			{
				VoiceComp->SetCurrentMeetingID(CurrentMeetingSessionID);
				VoiceComp->StartRecording();

				if (owner && owner->ChatComp && owner->ChatComp->GroupChatUI)
				{
					owner->ChatComp->GroupChatUI->OnRecordBtnState(true);
				}

				UE_LOG(LogTemp, Warning, TEXT(">>> [HOST] Start Recording Immediately (Skip Join)"));
			}
		}
		
		if (owner && owner->ChatComp && owner->ChatComp->GroupChatUI)
		{
			owner->SetIgnoreMoveInput(true);
			owner->Server_TrySitAtCampfire();
			
			UGroupChatUI* ChatUI = owner->ChatComp->GroupChatUI;

			// 1. "회의 시작" 알림 메시지 (나눔이가 말함)
			ChatUI->AddBotChat(TEXT("회의가 시작되었습니다. 이동이 제한되며 내용은 자동으로 기록됩니다."));

			// 2. 만약 AI 도우미가 켜져 있었다면 강제로 끔 (회의 중 충돌 방지)
			if (ChatUI->bIsMeetingChatbotActive)
			{
				ChatUI->OnAICheckStateChanged(false); // 웹소켓 종료 및 상태 false
				ChatUI->UpdateQuestionButtonState();  // 버튼 색상 원래대로
			}
            
			// 3. 녹음 버튼 UI 켜기 (이전 질문에서 추가한 부분)
			ChatUI->OnRecordBtnState(true);
		}
		// 3. 다른 팀원들에게만 Join 명령 내리기 위해 RPC 호출
		AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>();
		if (PS)
		{
			Server_RegisterMeetingState(PS->VoiceChannelID, MeetingID);
			Server_BroadcastJoinMeeting(PS->VoiceChannelID, MeetingID);

			Server_UpdateMeetingStatus(PS->VoiceChannelID, true);
		}
	}
	else
	{
		//owner->SetIgnoreMoveInput(true);
		//owner->Server_TrySitAtCampfire();
		
		UE_LOG(LogTemp, Error, TEXT("[Meeting] Failed to Create Meeting."));
	}
}

void UPlayerMeetingManagerComponent::OnJoinMeetingResponse(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("[Meeting] Joined Successfully! Starting Recording..."));

		// [최종] 녹음 컴포넌트 실행
		if (player)
		{
			if (UVoiceChatComponent* VoiceComp = player->FindComponentByClass<UVoiceChatComponent>())
			{
				// 녹음 시작 시 MeetingID를 전달해줘야 함 (VoiceComponent 수정 필요)
				VoiceComp->SetCurrentMeetingID(CurrentMeetingSessionID);
				VoiceComp->StartRecording();

				if (owner->IsLocalController())
				{
					owner->ChatComp->GroupChatUI->OnRecordBtnState(true);
					owner->Server_TrySitAtCampfire();
				}
			}
		}

		if (owner && owner->ChatComp && owner->ChatComp->GroupChatUI)
		{
			owner->SetIgnoreMoveInput(true);
			
			UGroupChatUI* ChatUI = owner->ChatComp->GroupChatUI;

			// 1. 알림 메시지
			ChatUI->AddBotChat(TEXT("회의에 참여했습니다. 이동이 제한되며 내용은 자동으로 기록됩니다."));

			// 2. AI 도우미 강제 종료
			if (ChatUI->bIsMeetingChatbotActive)
			{
				ChatUI->OnAICheckStateChanged(false);
				ChatUI->UpdateQuestionButtonState();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Meeting] Failed to Join Meeting."));
	}
}

void UPlayerMeetingManagerComponent::Server_RegisterMeetingState_Implementation(const FString& ChannelID,
	const FString& MeetingID)
{
	if (owner->GS)
	{
		owner->GS->RegisterMeeting(ChannelID, MeetingID);
	}
}

void UPlayerMeetingManagerComponent::Server_UnregisterMeetingState_Implementation(const FString& ChannelID)
{
	if (owner->GS)
	{
		owner->GS->UnregisterMeeting(ChannelID);
	}
}

