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
		DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, 0, 1.f);
		bool bIsHit = GetWorld()->LineTraceSingleByChannel(
			HitRes,
			Start,
			End,
			ECC_Visibility,
			CollisionParams
		);

		FTransform HitPointTransform(HitRes.ImpactNormal.Rotation() + FRotator(-90.f, 0.f, 0.f),
		                             HitRes.ImpactPoint + FVector(0.f, 0.f, 81.f), FVector::OneVector);
		PreviewTent->SetActorTransform(HitPointTransform);

		if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
		{
			OnClick(HitRes.ImpactPoint + FVector(0.f, 0.f, 81.f), HitRes.ImpactNormal.Rotation() + FRotator(-90.f, 0.f, 0.f));
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
	// Install Tent
	if (PreviewTent)
	{
		PreviewTent->Destroy();
		PreviewTent = nullptr;

		if (Tent)
		{
			// Move Tent
			Tent->SetActorLocationAndRotation(TentLocation, TentRotation);
		}
		else
		{
			// Spawn Tent
			// Tent = GetWorld()->SpawnActor<ATentActor>(
			// 	TentClass,
			// 	TentLocation,
			// 	TentRotation
			// );
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
	CurPlayer->PlayAlienDance();
	

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
		GM->SpawnTent(TentTransform);
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

                // [핵심 1] 상대방의 고유 ID(스팀 ID)가 유효한지 먼저 확인
                if (!OtherPS->GetUniqueId().IsValid())
                {
                    UE_LOG(LogTemp, Warning, TEXT("[Voice] ID Invalid for %s. Retrying in 0.5s..."), *OtherPS->GetPlayerName());
                    
                    // ID가 없으면 0.5초 뒤에 이 함수 전체를 다시 실행해서 재시도하게 함
                    FTimerHandle RetryHandle;
                    GetWorld()->GetTimerManager().SetTimer(RetryHandle, this, &ACuteAlienController::UpdateVoiceChannelMuting, 0.5f, false);
                    return; // 루프 중단하고 나중에 다시 함
                }

                // [핵심 2] ID가 확실히 있을 때만 뮤트/언뮤트 실행
            	if (OtherPS->GetUniqueId().IsValid()) 
            	{
            		// 1. 타입 변환: FUniqueNetIdRepl -> FUniqueNetId
            		// (* 연산자가 안전하게 변환해줌)
            		const FUniqueNetId& PlayerUniqueId = *OtherPS->GetUniqueId();

            		if (AlienOtherPS->VoiceChannelID == MyChannelID)
            		{
            			// 같은 채널 -> 언뮤트 (이미 뮤트된 경우만)
            			if (IsPlayerMuted(PlayerUniqueId)) 
            			{
            				GameplayUnmutePlayer(OtherPS->GetUniqueId());
            				UE_LOG(LogTemp, Log, TEXT(">>> [UNMUTE] %s (Same Channel %d)"), *OtherPS->GetPlayerName(), MyChannelID);
            			}
            		}
            		else
            		{
            			// 다른 채널 -> 뮤트 (뮤트 안 된 경우만)
            			if (!IsPlayerMuted(PlayerUniqueId)) 
            			{
            				GameplayMutePlayer(OtherPS->GetUniqueId());
            				UE_LOG(LogTemp, Log, TEXT(">>> [MUTE] %s (Other Channel %d)"), *OtherPS->GetPlayerName(), AlienOtherPS->VoiceChannelID);
            			}
            		}
            	}
            	else
            	{
            		UE_LOG(LogTemp, Warning, TEXT("ID Invalid yet..."));
            	}
            }
        }
    }
}
