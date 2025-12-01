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
#include "Yeomin/Network/DebugUtils.h"
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
		if (GI->PlayerUniqueID == 0) 
		{
			UE_LOG(LogTemp, Warning, TEXT("[Test] Detected Direct Level Start! Injecting Dummy Data..."));
            
			GI->PlayerUniqueID = 999 + GetWorld()->GetGameState()->PlayerArray.Num();       // 테스트 ID
			GI->PlayerName = FString::Printf(TEXT("EditorTester : %d"), GetWorld()->GetGameState()->PlayerArray.Num());
			GI->PlayerType = TEXT("운영진"); // 테스트용 권한
			GI->CampID = 1;
			GI->PlayerTendency = 0;
		}

		// [전송] 확정된 데이터를 서버로 1회 전송
		Server_InitPlayerInfo(
			GI->PlayerUniqueID,
			GI->PlayerName,
			GI->PlayerType,
			GI->PlayerTendency
		);
        
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
}

void ACuteAlienController::Server_InitPlayerInfo_Implementation(int32 UID, const FString& Name, const FString& Type, int32 Tendency)
{
	AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();
	if (PS)
	{
		PS->PS_UserIndex = UID;
		PS->SetPlayerName(Name);
		PS->PS_UserType = Type;
		PS->PS_TendencyID = Tendency;
		// PS->CampID = CampID; (인자 추가 시)
        
		// 강제 동기화 (선택)
		PS->ForceNetUpdate(); 
       
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
	if (MyPS)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Req] Start Recording Requested by: %d (Ch: %d)"), MyPS->PS_UserIndex, MyPS->VoiceChannelID);

		Server_StartChannelRecording(MyPS->VoiceChannelID);
	}
}

void ACuteAlienController::RequestStopMeetingRecording()
{
	AMumulPlayerState* MyPS = GetPlayerState<AMumulPlayerState>();
	if (MyPS)
	{
		Server_StopChannelRecording(MyPS->VoiceChannelID);
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


void ACuteAlienController::Server_RequestGroupChatUI_Implementation(const TArray<int32>& Players)
{
	// Add GroupChatUI for each Client
	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
	{
		if (Players.Contains(Cast<AMumulPlayerState>(PS)->PS_UserIndex))
		{
			ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController());
			if (PC)
			{
				PC->Client_CreateGroupChatUI(Players);
			}
		}
	}
}

void ACuteAlienController::Client_CreateGroupChatUI_Implementation(const TArray<int32>& Players)
{
	// Set Players in Group Icon
	UGroupIconUI* GroupIconUI = CreateWidget<UGroupIconUI>(GetWorld(), GroupIconUIClass);
	GroupIconUI->InitParentUI(GroupChatUI);
	GroupChatUI->AddGroupIcon(GroupIconUI);
	GroupIconUI->ChatBlockUI->SetPlayersInGroup(Players);
}


void ACuteAlienController::Server_RequestChat_Implementation(const TArray<int32>& Players, const FString& Text, const FString& Name, const FString& CurrentTime)
{
	// Add GroupChatUI for each Client
	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
	{
		if (Players.Contains(Cast<AMumulPlayerState>(PS)->PS_UserIndex))
		{
			ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController());
			if (PC)
			{
				PC->Client_SendChat(Text, Name, CurrentTime);
			}
		}
	}
}

void ACuteAlienController::Client_SendChat_Implementation(const FString& Text, const FString& Name, const FString& CurrentTime)
{
	GroupChatUI->AddChat(Text, Name, CurrentTime);
}
