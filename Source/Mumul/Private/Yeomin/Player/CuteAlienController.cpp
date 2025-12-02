// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Player/CuteAlienController.h"

#include "Yeomin/Player/CuteAlienPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Yeomin/UI/RadialUI.h"
#include "Blueprint/UserWidget.h"
#include "InputMappingContext.h"
#include "MumulGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "khc/Player/MumulPlayerState.h"
#include "MumulMumulGameMode.h"
#include "Yeomin/Tent/PreviewTentActor.h"
#include "Yeomin/Tent/TentActor.h"
#include "Net/VoiceConfig.h"
#include <khc/Player/VoiceChatComponent.h>

#include "HttpNetworkSubsystem.h"
#include "Base/MumulGameState.h"
#include "khc/Save/MapDataSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Yeomin/UI/ChatBlockUI.h"
#include "Yeomin/UI/GroupChatUI.h"
#include "Yeomin/UI/GroupIconUI.h"
#include "Yeomin/UI/PlayerUI.h"

ACuteAlienController::ACuteAlienController()
{
	static ConstructorHelpers::FClassFinder<URadialUI> RadialUIFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/WBP_RadialUI.WBP_RadialUI_C"));
	if (RadialUIFinder.Succeeded())
	{
		RadialUIClass = RadialUIFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UPlayerUI> PlayerUIFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/WBP_PlayerUI.WBP_PlayerUI_C"));
	if (PlayerUIFinder.Succeeded())
	{
		PlayerUIClass = PlayerUIFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UGroupChatUI> GroupChatUIFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/WBP_GroupChatUI.WBP_GroupChatUI_C"));
	if (GroupChatUIFinder.Succeeded())
	{
		GroupChatUIClass = GroupChatUIFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UGroupIconUI> GroupIconUIFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/WBP_GroupProfileUI.WBP_GroupProfileUI_C"));
	if (GroupIconUIFinder.Succeeded())
	{
		GroupIconUIClass = GroupIconUIFinder.Class;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/IMC_Player.IMC_Player"));
	if (IMCFinder.Succeeded())
	{
		IMC_Player = IMCFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_RadialFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_Radial.IA_Radial"));
	if (IA_RadialFinder.Succeeded())
	{
		IA_Radial = IA_RadialFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_CancelFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_Cancel.IA_Cancel"));
	if (IA_CancelFinder.Succeeded())
	{
		IA_Cancel = IA_CancelFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_ToggleMouseFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_ToggleMouse.IA_ToggleMouse"));
	if (IA_ToggleMouseFinder.Succeeded())
	{
		IA_ToggleMouse = IA_ToggleMouseFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_ClickFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_Click.IA_Click"));
	if (IA_ClickFinder.Succeeded())
	{
		IA_Click = IA_ClickFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_QuitGameFinder(
	TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_QuitGame.IA_QuitGame"));
	if (IA_QuitGameFinder.Succeeded())
	{
		IA_QuitGame = IA_QuitGameFinder.Object;
	}

	static ConstructorHelpers::FClassFinder<APreviewTentActor> PreviewTentFinder(
		TEXT("/Game/Yeomin/Actors/Tent/BP_PreviewTent.BP_PreviewTent_C"));
	if (PreviewTentFinder.Succeeded())
	{
		PreviewTentClass = PreviewTentFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<ATentActor> TentFinder(
		TEXT("/Game/Yeomin/Actors/Tent/BP_Tent.BP_Tent_C"));
	if (TentFinder.Succeeded())
	{
		TentClass = TentFinder.Class;
	}

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
}

void ACuteAlienController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
		return;
	
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(IMC_Player, 0);
	}

	RadialUI = CreateWidget<URadialUI>(this, RadialUIClass);
	RadialUI->AddToViewport();
	PlayerUI = CreateWidget<UPlayerUI>(this, PlayerUIClass);
	PlayerUI->AddToViewport();
	GroupChatUI = CreateWidget<UGroupChatUI>(this, GroupChatUIClass);
	GroupChatUI->AddToViewport();
	
	RadialUI->SetVisibility(ESlateVisibility::Hidden);

	// 4. 데이터 초기화 및 서버 전송
	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	if (GI)
	{
		// [체크] 로비 스킵 여부 확인 (데이터가 비어있으면 더미 데이터 주입)
		if (GI->PlayerUniqueID == 100) 
		{
			UE_LOG(LogTemp, Warning, TEXT("[Test] Detected Direct Level Start! Injecting Dummy Data..."));

			GI->PlayerUniqueID = 99 + GetWorld()->GetGameState()->PlayerArray.Num();       // 테스트 ID
			GI->PlayerName = GI->PlayerName + FString::FromInt(GI->PlayerUniqueID); // 이름 + index
			GI->CampID = 1;           // 임시 캠프 ID
			GI->PlayerType = (GI->PlayerUniqueID == 100) ? TEXT("운영진") : TEXT("학생");
			GI->PlayerTendency = 0;
			GI->bHasSurveyCompleted = true;
		}

		// [전송] 확정된 데이터를 서버로 1회 전송
		Server_InitPlayerInfo(
			GI->PlayerUniqueID,
			GI->PlayerName,
			GI->PlayerType,
			GI->PlayerTendency
		);

		if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			// [추가] HTTP 응답 바인딩
			HttpSystem->OnStartMeeting.AddDynamic(this, &ACuteAlienController::OnStartMeetingResponse);
			HttpSystem->OnJoinMeeting.AddDynamic(this, &ACuteAlienController::OnJoinMeetingResponse);
		}
        
		UE_LOG(LogTemp, Log, TEXT("[Client] Sent Init Info: %s (ID: %d)"), *GI->PlayerName, GI->PlayerUniqueID);
	}
}

void ACuteAlienController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(InputComponent);

	Input->BindAction(IA_Radial, ETriggerEvent::Started, this, &ACuteAlienController::ShowRadialUI);
	Input->BindAction(IA_Radial, ETriggerEvent::Completed, this, &ACuteAlienController::HideRadialUI);
	Input->BindAction(IA_Cancel, ETriggerEvent::Started, this, &ACuteAlienController::OnCancelUI);
	Input->BindAction(IA_ToggleMouse, ETriggerEvent::Started, this, &ACuteAlienController::OnToggleMouse);
	Input->BindAction(IA_QuitGame, ETriggerEvent::Started, this, &ACuteAlienController::OnPressEsc);
}

void ACuteAlienController::Server_InitPlayerInfo_Implementation(int32 UID, const FString& Name, const FString& Type, int32 Tendency)
{
	AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();
	if (PS)
	{
		PS->PS_UserIndex = UID;
		PS->SetPlayerName(Name);
		PS->PS_RealName = Name;
		PS->PS_UserType = Type;
		PS->PS_TendencyID = Tendency;
		// PS->CampID = CampID; (인자 추가 시)
        
		// 강제 동기화 (선택)
		PS->ForceNetUpdate();

		FString SlotName = TEXT("IslandMapSave");
		if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			UMapDataSaveGame* LoadInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
           
			// 해당 유저의 저장된 위치가 있는지 확인
			if (LoadInst && LoadInst->PlayerLocations.Contains(UID))
			{
				FTransform SavedTr = LoadInst->PlayerLocations[UID];
               
				// 폰 이동 (텔레포트)
				if (APawn* MyPawn = GetPawn())
				{
					// Z축(높이)을 살짝 띄워주는 게 안전합니다 (바닥 끼임 방지)
					FVector SafeLoc = SavedTr.GetLocation() + FVector(0, 0, 50.0f);
					SavedTr.SetLocation(SafeLoc);

					MyPawn->SetActorTransform(SavedTr, false, nullptr, ETeleportType::TeleportPhysics);
                   
					UE_LOG(LogTemp, Warning, TEXT("[Server] Restored User %d Location to %s"), UID, *SafeLoc.ToString());
				}
			}
		}
       
		UE_LOG(LogTemp, Log, TEXT("[Server] PlayerState Initialized: %s (ID: %d)"), *Name, UID);
	}
}

void ACuteAlienController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PreviewTent)
	{
		// Line Trace from ViewPoint
		FHitResult HitRes;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		FVector Start, End;
		FRotator CamRot;
		float Dist = 1500.f;
		GetPlayerViewPoint(Start, CamRot);
		End = Start + CamRot.Vector() * Dist;
		
		bool bIsHit = GetWorld()->LineTraceSingleByChannel(
			HitRes,
			Start,
			End,
			ECC_Visibility,
			CollisionParams
		);

		FTransform HitPointTransform(HitRes.ImpactNormal.Rotation() + FRotator(-90.f, 0.f, 0.f),
		                             HitRes.ImpactPoint, FVector::OneVector);
		PreviewTent->SetActorTransform(HitPointTransform);

		if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
		{
			OnClick(HitRes.ImpactPoint,
			        HitRes.ImpactNormal.Rotation() + FRotator(-90.f, 0.f, 0.f));
		}
	}
}

void ACuteAlienController::OnPressEsc()
{
	Server_SaveAndExit();
}

void ACuteAlienController::Server_SaveAndExit_Implementation()
{
	// 1. 저장 (아직 폰과 연결되어 있으므로 안전함)
	if (AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>())
	{
		if (APawn* MyPawn = GetPawn())
		{
			if (AMumulGameState* GS = GetWorld()->GetGameState<AMumulGameState>())
			{
				GS->Multicast_SavePlayerLocation(PS->PS_UserIndex, MyPawn->GetActorTransform());
				UE_LOG(LogTemp, Warning, TEXT("[Exit] Saved Location for User %d"), PS->PS_UserIndex);
			}
		}
	}

	// 2. 저장 후 종료 처리 (방장은 맵 이동, 클라이언트는 접속 종료)
	// 상황에 맞게 선택하세요.
    
	// Case A: 아예 게임 끄기 (Quit)
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false); 
    
	// Case B: 로비(메인 메뉴)로 돌아가기
	// if (HasAuthority()) // 방장이라면
	// {
	// 	// 방장이 나가면 다 같이 튕기거나 로비로 이동
	// 	GetWorld()->ServerTravel("/Game/Khc/Maps/Main?listen"); 
	// }
	// else // 클라이언트라면
	// {
	// 	ClientTravel("/Game/Khc/Maps/Main", TRAVEL_Absolute);
	// }
}

void ACuteAlienController::OnCancelUI()
{
	CancelRadialUI();

	if (PreviewTent)
	{
		PreviewTent->Destroy();
		PreviewTent = nullptr;
	}
}

void ACuteAlienController::OnToggleMouse()
{
	if (bShowMouseCursor)
	{
		SetIgnoreLookInput(false);
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
		return;
	}
	OnCancelUI();
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetIgnoreLookInput(true);
	SetShowMouseCursor(true);
	SetInputMode(InputMode);
}

void ACuteAlienController::OnClick(const FVector& TentLocation, const FRotator& TentRotation)
{
	// Place Tent
	if (PreviewTent)
	{
		// If Tent is Placeable
		if (PreviewTent->bIsPlaceable)
		{
			PreviewTent->Destroy();
			PreviewTent = nullptr;

			// Spawn or Move Tent
			Server_SpawnTent(FTransform(TentRotation, TentLocation));
		}
	}
}

void ACuteAlienController::ShowRadialUI()
{
	// Init UI
	OnCancelUI();

	// Lock Look Rotation
	SetIgnoreLookInput(true);
	// Hide Mouse Cursor
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

	//Show Radial UI
	RadialUI->SetVisibility(ESlateVisibility::Visible);
	bIsRadialVisible = true;
}

void ACuteAlienController::HideRadialUI()
{
	if (RadialUI->GetVisibility() == ESlateVisibility::Hidden)
		return;

	SetIgnoreLookInput(false);
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

	/* TODO: 임시 확인용 */
	ACuteAlienPlayer* CurPlayer = Cast<ACuteAlienPlayer>(GetPawn());
	CurPlayer->Server_PlayAlienDance();


	RadialUI->SetVisibility(ESlateVisibility::Hidden);
	bIsRadialVisible = false;
}

void ACuteAlienController::CancelRadialUI()
{
	SetIgnoreLookInput(false);

	RadialUI->SetVisibility(ESlateVisibility::Hidden);
	bIsRadialVisible = false;
}

void ACuteAlienController::ShowPreviewTent()
{
	// Deactivate Mouse Cursor
	SetIgnoreLookInput(false);
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

	// Spawn Preview Tent
	PreviewTent = GetWorld()->SpawnActor<APreviewTentActor>(
		PreviewTentClass,
		GetPawn()->GetActorLocation(),
		FRotator::ZeroRotator
	);
}

void ACuteAlienController::RequestStartMeetingRecording()
{
	AMumulPlayerState* MyPS = GetPlayerState<AMumulPlayerState>();
	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
    
	if (MyPS && GI)
	{
		int32 ChannelID = MyPS->VoiceChannelID;
        
		// [HTTP] 방장(Organizer)이 Start Meeting API 호출
		// (서버 응답이 오면 OnStartMeetingResponse가 실행됨)
		if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			// TODO: 제목, 안건 등은 UI에서 입력받거나 기본값 사용
			HttpSystem->StartMeetingRequest(
				TEXT("Team Meeting Test"), 
				GI->PlayerUniqueID, // Organizer ID
				TEXT("Agenda Test"), 
				TEXT("Description Test")
			);
            
			UE_LOG(LogTemp, Log, TEXT("[Meeting] Requesting Start Meeting API..."));
		}
	}
}

void ACuteAlienController::RequestStopMeetingRecording()
{
	AMumulPlayerState* MyPS = GetPlayerState<AMumulPlayerState>();
	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());

	if (MyPS && GI)
	{
		// 1. 모든 클라이언트에게 녹음 중지 명령 (RPC) -> 각자 마지막 청크 전송
		Server_StopChannelRecording(MyPS->VoiceChannelID);

		// 2. [추가] 서버에 회의 종료 알림 (API 호출)
		if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			// 저장해둔 CurrentMeetingSessionID 사용
			if (!CurrentMeetingSessionID.IsEmpty())
			{
				HttpSystem->EndMeetingRequest(CurrentMeetingSessionID);
			}
		}
	}
}

void ACuteAlienController::Server_StartChannelRecording_Implementation(int32 TargetChannelID)
{
	UE_LOG(LogTemp, Warning, TEXT("[Server] Request Start for Ch: %d"), TargetChannelID);


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
						TargetPC->Client_StartChannelRecording(TargetChannelID);

						UE_LOG(LogTemp, Log, TEXT("[Server] Sent Start Command to: %s"), *PS->GetPlayerName());
					}
				}
			}
		}
	}
}

void ACuteAlienController::Client_StartChannelRecording_Implementation(int32 TargetChannelID)
{
	APawn* MyPawn = GetPawn();
	if (MyPawn)
	{
		if (UVoiceChatComponent* VoiceComp = MyPawn->FindComponentByClass<UVoiceChatComponent>())
		{
			VoiceComp->StartRecording(); // 실제 녹음 시작

			UE_LOG(LogTemp, Warning, TEXT(">>> [RECORD START] MeetingID: %d"), TargetChannelID);
		}
	}
}

void ACuteAlienController::Server_StopChannelRecording_Implementation(int32 TargetChannelID)
{
	UE_LOG(LogTemp, Warning, TEXT("[Server] Request Stop for Ch: %d"), TargetChannelID);

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
						TargetPC->Client_StopChannelRecording();
					}
				}
			}
		}
	}
}

void ACuteAlienController::Client_StopChannelRecording_Implementation()
{
	APawn* MyPawn = GetPawn();
	if (MyPawn)
	{
		if (UVoiceChatComponent* VoiceComp = MyPawn->FindComponentByClass<UVoiceChatComponent>())
		{
			VoiceComp->StopRecording(); // 녹음 종료

			UE_LOG(LogTemp, Warning, TEXT(">>> [RECORD STOP]"));
		}
	}
}

void ACuteAlienController::Server_SpawnTent_Implementation(const FTransform& TentTransform)
{
	AMumulMumulGameMode* GM = GetWorld()->GetAuthGameMode<AMumulMumulGameMode>();
	if (GM)
	{
		AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();
		if (PS)
		{
			// [수정] UserIndex도 같이 넘기고, bSaveToDisk = true로 설정
			GM->SpawnTent(TentTransform, PS->PS_UserIndex, true);
		}
	}
}

void ACuteAlienController::Server_BroadcastJoinMeeting_Implementation(int32 TargetChannelID, const FString& MeetingID)
{
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			for (APlayerState* PS : GameState->PlayerArray)
			{
				AMumulPlayerState* MumulPS = Cast<AMumulPlayerState>(PS);
				if (MumulPS && MumulPS->VoiceChannelID == TargetChannelID)
				{
					if (ACuteAlienController* TargetPC = Cast<ACuteAlienController>(PS->GetOwner()))
					{
						// [Client RPC] "야, 이 미팅 ID로 API 쏴서 참가해"
						TargetPC->Client_RequestJoinMeeting(MeetingID);
					}
				}
			}
		}
	}
}

void ACuteAlienController::Client_RequestJoinMeeting_Implementation(const FString& MeetingID)
{
	CurrentMeetingSessionID = MeetingID;

	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	if (GI)
	{
		if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			// [HTTP] Join Meeting API 호출
			HttpSystem->JoinMeetingRequest(GI->PlayerUniqueID, MeetingID);
		}
	}
}

void ACuteAlienController::OnStartMeetingResponse(bool bSuccess, FString MeetingID)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("[Meeting] Created Successfully: %s"), *MeetingID);
        
		// 방장이니까 자기 자신은 Join 절차 생략하고 바로 ID 저장
		CurrentMeetingSessionID = MeetingID;
        
		AMumulPlayerState* MyPS = GetPlayerState<AMumulPlayerState>();
		if (MyPS)
		{
			// [Server RPC] 같은 채널 사람들에게 Join 명령 내림
			Server_BroadcastJoinMeeting(MyPS->VoiceChannelID, MeetingID);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Meeting] Failed to Create Meeting."));
	}
}

void ACuteAlienController::OnJoinMeetingResponse(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("[Meeting] Joined Successfully! Starting Recording..."));

		// [최종] 녹음 컴포넌트 실행
		if (APawn* MyPawn = GetPawn())
		{
			if (UVoiceChatComponent* VoiceComp = MyPawn->FindComponentByClass<UVoiceChatComponent>())
			{
				// 녹음 시작 시 MeetingID를 전달해줘야 함 (VoiceComponent 수정 필요)
				VoiceComp->SetCurrentMeetingID(CurrentMeetingSessionID);
				VoiceComp->StartRecording();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Meeting] Failed to Join Meeting."));
	}
}

void ACuteAlienController::UpdateVoiceChannelMuting()
{
    AMumulPlayerState* MyPS = GetPlayerState<AMumulPlayerState>();
    if (!MyPS) return;

    int32 MyChannelID = MyPS->VoiceChannelID;

    if (UWorld* World = GetWorld())
    {
        if (AGameStateBase* GameState = World->GetGameState())
        {
            for (APlayerState* OtherPS : GameState->PlayerArray)
            {
                if (OtherPS == MyPS) continue;

                AMumulPlayerState* AlienOtherPS = Cast<AMumulPlayerState>(OtherPS);
                if (!AlienOtherPS) continue;

                // [핵심 수정] 무조건 Create 하지 말고, Get으로 먼저 찾습니다.
                UVOIPTalker* Talker = UVOIPStatics::GetVOIPTalkerForPlayer(OtherPS->GetUniqueId());
                
                // 없으면 그때 생성
                if (!Talker)
                {
                    Talker = UVOIPTalker::CreateTalkerForPlayer(OtherPS);
                }
                
                if (Talker)
                {
                   if (AlienOtherPS->VoiceChannelID == MyChannelID)
                   {
                      // [0번 채널] 3D 거리 기반
                      if (MyChannelID == 0)
                      {
                         Talker->Settings.AttenuationSettings = NormalAttenuation;
                            
                         if (APawn* OtherPawn = OtherPS->GetPawn())
                         {
                            Talker->Settings.ComponentToAttachTo = OtherPawn->GetRootComponent();
                         }
                         else
                         {
                            // 폰이 안 보이면(멀리 있으면) 소리 위치를 잡을 수 없음 -> 2D로 들리거나 안 들릴 수 있음
                            // 확실히 하기 위해 폰이 없으면 소리를 끄는 것도 방법
                             Talker->Settings.ComponentToAttachTo = nullptr;
                         }
                      }
                      // [그 외] 2D 전체
                      else
                      {
                         Talker->Settings.AttenuationSettings = nullptr;
                         Talker->Settings.ComponentToAttachTo = nullptr;
                      }
                   }
                   else
                   {
                      // [다른 채널] 무음
                      if (SilentAttenuation)
                      {
                         Talker->Settings.AttenuationSettings = SilentAttenuation;
                         Talker->Settings.ComponentToAttachTo = nullptr;
                      }
                   }
                }
            }
        }
    }
}


void ACuteAlienController::Server_RequestGroupChatUI_Implementation(const FString& GroupName, const TArray<int32>& Players)
{
	// Add GroupChatUI for each Client
	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
	{
		if (Players.Contains(Cast<AMumulPlayerState>(PS)->PS_UserIndex))
		{
			ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController());
			if (PC)
			{
				PC->Client_CreateGroupChatUI(GroupName, Players);
			}
		}
	}
}

void ACuteAlienController::Client_CreateGroupChatUI_Implementation(const FString& GroupName, const TArray<int32>& Players)
{
	
	// Set Players in Group Icon
	UGroupIconUI* GroupIconUI = CreateWidget<UGroupIconUI>(GetWorld(), GroupIconUIClass);
	GroupIconUI->InitParentUI(GroupChatUI);
	GroupChatUI->AddGroupIcon(GroupIconUI);
	GroupIconUI->ChatBlockUI->SetPlayersInGroup(Players);
	GroupIconUI->ChatBlockUI->SetGroupName(GroupName);
}


void ACuteAlienController::Server_RequestChat_Implementation(const TArray<int32>& Players, const FString& CurrentTime, const FString& Name, const FString& Text)
{
	// Add GroupChatUI for each Client
	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
	{
		if (Players.Contains(Cast<AMumulPlayerState>(PS)->PS_UserIndex))
		{
			ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController());
			if (PC)
			{
				PC->Client_SendChat(CurrentTime, Name, Text);
			}
		}
	}
}

void ACuteAlienController::Client_SendChat_Implementation(const FString& CurrentTime, const FString& Name, const FString& Text)
{
	GroupChatUI->AddChat(CurrentTime, Name, Text);
}
