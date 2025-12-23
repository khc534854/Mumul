// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CuteAlienController.h"

#include "EngineUtils.h"
#include "Player/CuteAlienPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UI/RadialUI.h"
#include "Blueprint/UserWidget.h"
#include "InputMappingContext.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Base/MumulGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "Player/MumulPlayerState.h"
#include "Object/Tent/PreviewTentActor.h"
#include "Net/VoiceConfig.h"
#include "PCGComponent.h"

#include "Network/HttpNetworkSubsystem.h"
#include "Base/MumulGameState.h"
#include "Components/WidgetSwitcher.h"
#include "Save/MapDataSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Data/IMGManager.h"
#include "Object/FeedbackObjectActor.h"
#include "Object/OXQuizTriggerActor.h"
#include "UI/PlayerUI.h"
#include "Player/Component/PlayerChatComponent.h"
#include "Player/Component/PlayerHousingSystemComponent.h"
#include "Player/Component/PlayerMeetingManagerComponent.h"
#include "Player/Component/PlayerOXQuizComponent.h"
#include "UI/FeedbackUI.h"
#include "UI/LogoutUI.h"
#include "Data/AudioManager.h"
#include "Network/WebSocketSubsystem.h"
#include "UI/GroupChatUI.h"
#include "Player/Component/PlayerNoticeComponent.h"

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

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_InteractFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_Interact.IA_Interact"));
	if (IA_InteractFinder.Succeeded())
	{
		IA_Interact = IA_InteractFinder.Object;
	}
	
	// Component
	HousingComp = CreateDefaultSubobject<UPlayerHousingSystemComponent>(TEXT("HousingComp"));
	MeetingComp = CreateDefaultSubobject<UPlayerMeetingManagerComponent>(TEXT("MeetingComp"));
	ChatComp = CreateDefaultSubobject<UPlayerChatComponent>(TEXT("SocialComp"));
	OXQuizComp = CreateDefaultSubobject<UPlayerOXQuizComponent>(TEXT("OXQuizComp"));
	NoticeComp = CreateDefaultSubobject<UPlayerNoticeComponent>(TEXT("NoticeComp"));

	static ConstructorHelpers::FClassFinder<UFeedbackUI> FeedbackUIFinder(
	   TEXT("/Game/Khc/Blueprint/UI/WBP_FeedbackUI.WBP_FeedbackUI_C")); // 경로 확인 필수!
	if (FeedbackUIFinder.Succeeded())
	{
		FeedbackUIClass = FeedbackUIFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<ULogoutUI> LogoutUIFinder(
   TEXT("/Game/Khc/Blueprint/UI/WBP_LogoutUI.WBP_LogoutUI_C")); // 경로 확인 필수!
	if (LogoutUIFinder.Succeeded())
	{
		LogoutUIClass = LogoutUIFinder.Class;
	}
}

void ACuteAlienController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		// 1. 도착하자마자 화면을 검게 유지 (로비에서 넘어온 페이드가 풀릴 수 있으므로 강제 설정)
		// Alpha 1.0 -> 1.0 (계속 검음)
		PlayerCameraManager->StartCameraFade(1.0f, 1.0f, 10.0f, FLinearColor::Black, true, true);

		// 2. 조작 및 UI 차단 (Cinematic Mode)
		// bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning
		SetCinematicMode(true, false, true, true, true);
        
		// 마우스 커서도 숨김
		bShowMouseCursor = false;
		SetIgnoreLookInput(true);
		SetIgnoreMoveInput(true);
	}

	GS = Cast<AMumulGameState>(GetWorld()->GetGameState());
	
	HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>();


	if (!IsLocalController())
		return;

	AudioManager = GetGameInstance()->GetSubsystem<UAudioManager>();
	if (AudioManager)
	{
		AudioManager->PlayIslandBGM();
	}
	
	IMGManager = NewObject<UIMGManager>(this, UIMGManager::StaticClass());

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(IMC_Player, 0);
	}

	if (RadialUIClass)
	{
		RadialUI = CreateWidget<URadialUI>(this, RadialUIClass);
		if (RadialUI)
		{
			RadialUI->AddToViewport();
			RadialUI->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (PlayerUIClass)
	{
		PlayerUI = CreateWidget<UPlayerUI>(this, PlayerUIClass);
		if (PlayerUI)
		{
			PlayerUI->AddToViewport(100);
			PlayerUI->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (PlayerUI && ChatComp->GroupChatUI)
	{
		PlayerUI->InitGroupChatUI(ChatComp->GroupChatUI);
		ChatComp->GroupChatUI->SetVisibility(ESlateVisibility::Hidden);
		RadialUI->SetVisibility(ESlateVisibility::Hidden);
	}

	TryInitPlayerInfo();

	// [수정] PlayerState가 준비될 때까지 타이머로 확인 (0.5초 간격)
	GetWorldTimerManager().SetTimer(InitPlayerStateTimerHandle, this, &ACuteAlienController::TryInitPlayerInfo, 0.5f,
	                                true);
}

void ACuteAlienController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(InputComponent);

	Input->BindAction(IA_Radial, ETriggerEvent::Started, this, &ACuteAlienController::ShowRadialUI);
	Input->BindAction(IA_Radial, ETriggerEvent::Completed, this, &ACuteAlienController::HideRadialUI);
	Input->BindAction(IA_Cancel, ETriggerEvent::Started, this, &ACuteAlienController::OnCancelUI);
	Input->BindAction(IA_ToggleMouse, ETriggerEvent::Started, this, &ACuteAlienController::OnToggleMouse);
	//Input->BindAction(IA_QuitGame, ETriggerEvent::Started, this, &ACuteAlienController::OnPressEsc);
	Input->BindAction(IA_Interact, ETriggerEvent::Started, this, &ACuteAlienController::OnInteract);
}

void ACuteAlienController::Server_InitPlayerInfo_Implementation(int32 UID, const FString& Name, const FString& Type,
                                                                int32 Tendency)
{
	AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();
	if (PS)
	{
		PS->PS_UserIndex = UID;
		if (IsLocalController())
		{
			PS->OnRep_UserIndex();
		}
		PS->SetPlayerName(Name);
		PS->PS_RealName = Name;
		PS->OnRep_RealName();
		PS->PS_UserType = Type;
		PS->Server_SetVoiceChannelID(TEXT("Lobby"));
		// PS->CampID = CampID; (인자 추가 시)

		PS->PS_TendencyID = Tendency;
		PS->OnRep_TendencyID();


		

		// 강제 동기화 (선택)
		PS->ForceNetUpdate();
		
		bool bIsFirstTime = true;

		FString SlotName = TEXT("IslandMapSave");
		if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			UMapDataSaveGame* LoadInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

			// 해당 유저의 저장된 위치가 있는지 확인
			if (LoadInst)
			{
				// Z축(높이)을 살짝 띄워주는 게 안전합니다 (바닥 끼임 방지)
				if (FTransform* SavedTr = LoadInst->PlayerLocations.Find(UID))
				{
					if (APawn* MyPawn = GetPawn())
					{
						bIsFirstTime = false;
						// Z축 보정 (바닥 끼임 방지)
						FVector SafeLoc = SavedTr->GetLocation() + FVector(0, 0, 50.0f);
						SavedTr->SetLocation(SafeLoc);
                   
						MyPawn->SetActorTransform(*SavedTr, false, nullptr, ETeleportType::TeleportPhysics);
                   
						UE_LOG(LogTemp, Warning, TEXT("[Server] Restored User %d Location to %s"), UID, *SafeLoc.ToString());
					}
				}

				if (FName* SavedItemID = LoadInst->PlayerCosmetics.Find(UID))
				{
					PS->EquippedCustomID = *SavedItemID;
					PS->OnRep_EquippedCustomID();
					UE_LOG(LogTemp, Log, TEXT("[Server] Loaded User %d Cosmetic: %s"), UID, *SavedItemID->ToString());
				}
			}
		}

		UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
		if (GS)
		{
			GS->Multicast_SavePlayerTendency(GI->PlayerUniqueID, GI->PlayerTendency);
		}
		
		if (bIsFirstTime)
		{
			Client_PlayLoadSequence();
			UE_LOG(LogTemp, Warning, TEXT("[Server] User %d is New! Requesting Intro Sequence."), UID);
		}
		
		UE_LOG(LogTemp, Log, TEXT("[Server] PlayerState Initialized: %s (ID: %d)"), *Name, UID);


	}
}


void ACuteAlienController::Client_PlayLoadSequence_Implementation()
{

	GetWorld()->GetTimerManager().SetTimer(
		PCGWaitTimerHandle, 
		this, 
		&ACuteAlienController::CheckPCGAndPlayIntro, 
		0.5f, 
		true
	);
	
	
}

void ACuteAlienController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bCanInteract)
	{
		if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
			TryInteractWithFeedbackActor();
	}

	if (bCanSit)
	{
		if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
			SitState(true);
	}
}

void ACuteAlienController::OnPressEsc()
{
	SaveAndExit();
}

void ACuteAlienController::OnInteract()
{
}

void ACuteAlienController::Server_RequestStartQuiz_Implementation(AOXQuizTriggerActor* QuizTrigger)
{
	QuizTrigger->OnTriggerQuiz(Cast<AMumulPlayerState>(PlayerState)->PS_UserIndex);
}

void ACuteAlienController::SaveAndExit()
{
	// 1. 저장 (아직 폰과 연결되어 있으므로 안전함)
	if (AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>())
	{
		if (APawn* MyPawn = GetPawn())
		{
			if (GS)
			{
				GS->Multicast_SavePlayerLocation(PS->PS_UserIndex, MyPawn->GetActorTransform());
				UE_LOG(LogTemp, Warning, TEXT("[Exit] Saved Location for User %d"), PS->PS_UserIndex);
			}
		}
	}
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

void ACuteAlienController::OnCancelUI()
{
	CancelRadialUI();
	
	if (HousingComp->PreviewTent)
	{
		if (PlayerUI)
			PlayerUI->CancelTent();
		
		HousingComp->PreviewTent->Destroy();
		HousingComp->PreviewTent = nullptr;
	}
	//PlayerUI->CloseSidePanels();

	HousingComp->StopPreviewHousingItem();

	if (AMumulCharacter* MyChar = Cast<AMumulCharacter>(GetPawn()))
	{
		MyChar->SetFirstPersonView(false);
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
	if (HousingComp->PreviewTent)
	{
		// If Tent is Placeable
		if (HousingComp->PreviewTent->bIsPlaceable)
		{
			PlayerUI->CloseSidePanels();
			
			HousingComp->PreviewTent->Destroy();
			HousingComp->PreviewTent = nullptr;

			// Spawn or Move Tent
			HousingComp->Server_SpawnTent(FTransform(TentRotation, TentLocation));
			Cast<ACuteAlienPlayer>(GetCharacter())->PlayTentSpawnSound();

			if (AMumulCharacter* MyChar = Cast<AMumulCharacter>(GetPawn()))
			{
				MyChar->SetFirstPersonView(false);
			}
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
	RadialUI->PlaySlotSequence();
	bIsRadialVisible = true;
}

void ACuteAlienController::HideRadialUI()
{
	if (!RadialUI || RadialUI->GetVisibility() == ESlateVisibility::Hidden)
		return;

	SetIgnoreLookInput(false);
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

	RadialUI->SetVisibility(ESlateVisibility::Hidden);
	bIsRadialVisible = false;

	// [추가] 선택된 슬롯 확인
	int32 SelectedIdx = RadialUI->GetCurrentSelectedIndex();
	UE_LOG(LogTemp, Log, TEXT("Selected Radial Slot: %d"), SelectedIdx);

	// 플레이어 가져오기
	ACuteAlienPlayer* CurPlayer = Cast<ACuteAlienPlayer>(GetPawn());
	if (!CurPlayer) return;

	CurPlayer->Server_PlayAlienDance(SelectedIdx);
}

void ACuteAlienController::CancelRadialUI()
{
	SetIgnoreLookInput(false);

	RadialUI->SetVisibility(ESlateVisibility::Hidden);
	bIsRadialVisible = false;
}

void ACuteAlienController::TryInteractWithFeedbackActor()
{
	FHitResult HitResult;
	FVector Start, End;
	FRotator CamRot;
    
	// 카메라 위치에서 시선 방향으로 레이저 발사
	GetPlayerViewPoint(Start, CamRot);
	float InteractionDistance = 800.0f; // 상호작용 가능 거리 (충분히 길게)
	End = Start + CamRot.Vector() * InteractionDistance;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	if (GetPawn()) Params.AddIgnoredActor(GetPawn());

	// ECC_Visibility 채널로 검사 (FeedbackActor의 Mesh가 이 채널을 Block해야 함)
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		FlushPressedKeys();
		// 맞은 물체가 피드백 액터인지 확인
		AFeedbackObjectActor* HitActor = Cast<AFeedbackObjectActor>(HitResult.GetActor());
		if (HitActor)
		{
			// UI가 아직 없으면 생성
			if (!FeedbackUI && FeedbackUIClass)
			{
				FeedbackUI = CreateWidget<UFeedbackUI>(this, FeedbackUIClass);
				FeedbackUI->AddToViewport(10); // 다른 UI보다 위에 오도록 Z-Order 설정
			}

			// UI 표시 및 입력 모드 전환
			if (FeedbackUI)
			{
				FeedbackUI->OpenFeedbackUI(); // UI 내부 초기화 함수 호출
                
				SetShowMouseCursor(true);
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(FeedbackUI->TakeWidget());
				SetInputMode(InputMode);
			}
		}
	}
}

void ACuteAlienController::SitState(bool newSitState)
{
	bIsSitting = newSitState;

	if (bIsSitting)
	{
		if (TargetChair)
		{
			// 플레이어 앉히기
		}
	}
	else
	{
		// 플레이어 때기
	}
}

void ACuteAlienController::OpenLogoutUI()
{
	FlushPressedKeys();
	// UI가 아직 없으면 생성
	if (!LogoutUI && LogoutUIClass)
	{
		LogoutUI = CreateWidget<ULogoutUI>(this, LogoutUIClass);
		if (LogoutUI)
		{
			LogoutUI->AddToViewport(20); // Z-Order를 높게 설정 (최상단)
		}
	}

	// UI 표시 및 입력 모드 전환
	if (LogoutUI)
	{
		// [핵심 수정] 숨겨져 있던 위젯을 다시 보이게 설정
		LogoutUI->SetVisibility(ESlateVisibility::Visible);

		SetShowMouseCursor(true);
       
		FInputModeUIOnly InputMode;
		// 위젯 포커스를 강제로 잡아서 ESC키 등이 UI에서 처리되도록 함
		InputMode.SetWidgetToFocus(LogoutUI->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
	}
}

void ACuteAlienController::TryInitPlayerInfo()
{
	AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();

	// PlayerState가 아직 없으면 다음 틱을 기다림
	if (!PS)
	{
		return;
	}

	// PlayerState가 유효하면 타이머 종료
	GetWorldTimerManager().ClearTimer(InitPlayerStateTimerHandle);

	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	if (GI)
	{
		// [체크] 로비 스킵 여부 확인 (테스트용)
		if (GI->PlayerUniqueID == 10)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Test] Detected Direct Level Start! Injecting Dummy Data..."));

			// GameState 확인 (안전장치)
			int32 CurrentPlayerCount = 0;
			if (AGameStateBase* CurrentGS = GetWorld()->GetGameState())
			{
				CurrentPlayerCount = CurrentGS->PlayerArray.Num();
			}

			GI->PlayerUniqueID = 9 + CurrentPlayerCount;
			GI->PlayerName = GI->PlayerName + FString::FromInt(GI->PlayerUniqueID);
			GI->CampID = 1;
			GI->PlayerType = (GI->PlayerUniqueID == 10) ? TEXT("운영진") : TEXT("학생");
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
		


		UWebSocketSubsystem* WS = GI->GetSubsystem<UWebSocketSubsystem>();
		if (WS)
		{
			if (!WS->IsConnected())
			{
				UE_LOG(LogTemp, Log, TEXT("[Client] TryInitPlayerInfo: Connecting WebSocket..."));
				WS->Connect();
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("[Client] WebSocket already connected."));
			}
		}

		UE_LOG(LogTemp, Log, TEXT("[Client] Sent Init Info: %s (ID: %d)"), *GI->PlayerName, GI->PlayerUniqueID);
	}
}

void ACuteAlienController::CheckPCGAndPlayIntro()
{
	bool bIsGenerating = false;

	// 1. 월드에 존재하는 모든 PCG 컴포넌트를 순회하며 생성 중인지 확인
	// (TObjectIterator는 메모리상의 모든 객체를 찾으므로 GetWorld() 체크 필수)
	for (TObjectIterator<UPCGComponent> It; It; ++It)
	{
		UPCGComponent* PCGComp = *It;
		if (PCGComp && PCGComp->GetWorld() == GetWorld())
		{
			
			if (PCGComp->IsGenerating())
			{
				bIsGenerating = true;
				break; // 하나라도 생성 중이면 즉시 중단하고 대기
			}
		}
	}

	if (bIsGenerating)
		return;

	// 타이머 종료 및 카운트 초기화
	GetWorld()->GetTimerManager().ClearTimer(PCGWaitTimerHandle);

	// 시네마틱 액터 찾기
	ALevelSequenceActor* TargetSeqActor = nullptr;
	for (TActorIterator<ALevelSequenceActor> It(GetWorld()); It; ++It)
	{
		if (It->GetSequencePlayer()) 
		{
			TargetSeqActor = *It;
			break;
		}
	}

	if (TargetSeqActor)
	{
		IntroSequencePlayer = TargetSeqActor->GetSequencePlayer();
		if (IntroSequencePlayer)
		{
			// 1. [중요] 시네마틱 종료 이벤트 바인딩
			IntroSequencePlayer->OnFinished.AddDynamic(this, &ACuteAlienController::OnIntroSequenceFinished);

			// 2. 재생 시작
			IntroSequencePlayer->Play();

			// 3. 화면 페이드 인 (검은 화면 -> 밝은 화면)
			// 1.0초에 걸쳐 서서히 밝아짐
			PlayerCameraManager->StartCameraFade(1.0f, 0.0f, 1.0f, FLinearColor::Black, false, true);
               
			UE_LOG(LogTemp, Warning, TEXT("[Intro] PCG Ready. Starting Sequence & Fade In."));
		}
	}
	else
	{
		// 시네마틱이 없다면 바로 게임 시작 처리
		OnIntroSequenceFinished();
	}
}

void ACuteAlienController::OnIntroSequenceFinished()
{
	SetCinematicMode(false, true, true, true, true);

	// 2. 입력 허용
	bShowMouseCursor = true;
	SetIgnoreLookInput(false);
	SetIgnoreMoveInput(false);
    
	// 마우스 모드 설정 (게임+UI)
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	if (PlayerUI)
	{
		PlayerUI->SetVisibility(ESlateVisibility::Visible);
	}

	if (ChatComp && ChatComp->GroupChatUI)
	{
		ChatComp->GroupChatUI->SetVisibility(ESlateVisibility::Visible);
	}
	
	if (PlayerCameraManager)
	{
		PlayerCameraManager->StopCameraFade();
	}
}

