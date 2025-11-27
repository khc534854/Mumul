// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Player/CuteAlienController.h"

#include "Yeomin/Player/CuteAlienPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Yeomin/UI/RadialUI.h"
#include "Blueprint/UserWidget.h"
#include "InputMappingContext.h"
#include "GameFramework/GameStateBase.h"
#include "khc/Player/MumulPlayerState.h"
#include "MumulMumulGameMode.h"
#include "Yeomin/Tent/PreviewTentActor.h"
#include "Yeomin/Tent/TentActor.h"
#include "Net/VoiceConfig.h"

ACuteAlienController::ACuteAlienController()
{
	static ConstructorHelpers::FClassFinder<URadialUI> RadialUIFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/WBP_RadialUI.WBP_RadialUI_C"));
	if (RadialUIFinder.Succeeded())
	{
		RadialUIClass = RadialUIFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> PlayerUIFinder(
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
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(IMC_Player, 0);
	}
	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(IMC_Player, 0);
		}

		RadialUI = CreateWidget<URadialUI>(this, RadialUIClass);
		RadialUI->AddToViewport();
		PlayerUI = CreateWidget<UUserWidget>(this, PlayerUIClass);
		PlayerUI->AddToViewport();

		RadialUI->SetVisibility(ESlateVisibility::Hidden);
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

void ACuteAlienController::Server_SpawnTent_Implementation(const FTransform& TentTransform)
{
	AMumulMumulGameMode* GM = GetWorld()->GetAuthGameMode<AMumulMumulGameMode>();
	if (GM)
	{
		GM->SpawnTent(TentTransform, GetPlayerState<AMumulPlayerState>()->GetPlayerName());
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
                if (OtherPS == MyPS) continue; // 나는 건너뜀

            	AMumulPlayerState* AlienOtherPS = Cast<AMumulPlayerState>(OtherPS);
            	if (!AlienOtherPS) continue;

            	// [핵심] 이 플레이어의 목소리를 담당하는 Talker 객체를 가져옵니다.
            	UVOIPTalker* Talker = UVOIPTalker::CreateTalkerForPlayer(OtherPS);
                
            	if (Talker)
            	{
            		// 1. 채널이 같은가?
            		if (AlienOtherPS->VoiceChannelID == MyChannelID)
            		{
            			// [상황 A] 둘 다 0번(로비) 채널 -> 거리 기반(3D) 적용
            			if (MyChannelID == 0)
            			{
            				Talker->Settings.AttenuationSettings = NormalAttenuation;
                            
            				// [중요] 3D 사운드는 소리 나는 위치(상대방 캐릭터)를 지정해야 함
            				if (APawn* OtherPawn = OtherPS->GetPawn())
            				{
            					Talker->Settings.ComponentToAttachTo = OtherPawn->GetRootComponent();
            				}
                            
            				UE_LOG(LogTemp, Log, TEXT(">>> [VOICE 3D] %s (Proximity)"), *OtherPS->GetPlayerName());
            			}
            			// [상황 B] 둘 다 모닥불(특정) 채널 -> 전체 들림(2D) 적용
            			else
            			{
            				Talker->Settings.AttenuationSettings = nullptr; // 감쇠 없음 = 2D
            				Talker->Settings.ComponentToAttachTo = nullptr; // 위치 상관 없음
                            
            				UE_LOG(LogTemp, Log, TEXT(">>> [VOICE 2D] %s (Room Mode)"), *OtherPS->GetPlayerName());
            			}
            		}
            		// 2. 채널이 다른가?
            		else
            		{
            			// [상황 C] 다른 채널 -> 소리 소멸(Silent) 적용
            			if (SilentAttenuation)
            			{
            				Talker->Settings.AttenuationSettings = SilentAttenuation;
            				Talker->Settings.ComponentToAttachTo = nullptr;
            			}
            			UE_LOG(LogTemp, Log, TEXT(">>> [VOICE MUTE] %s"), *OtherPS->GetPlayerName());
            		}
            	}
            }
        }
    }
}