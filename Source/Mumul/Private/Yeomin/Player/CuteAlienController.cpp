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
//#include "Net/VoiceConfig.h"
#include "Yeomin/Tent/PreviewTentActor.h"

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

	RadialUI = CreateWidget<URadialUI>(this, RadialUIClass);
	RadialUI->AddToViewport();
	PlayerUI = CreateWidget<UUserWidget>(this, PlayerUIClass);
	PlayerUI->AddToViewport();

	RadialUI->SetVisibility(ESlateVisibility::Hidden);
}

void ACuteAlienController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(InputComponent);

	Input->BindAction(IA_Radial, ETriggerEvent::Started, this, &ACuteAlienController::ShowRadialUI);
	Input->BindAction(IA_Radial, ETriggerEvent::Completed, this, &ACuteAlienController::HideRadialUI);
	Input->BindAction(IA_Cancel, ETriggerEvent::Started, this, &ACuteAlienController::OnCancel);
	Input->BindAction(IA_ToggleMouse, ETriggerEvent::Started, this, &ACuteAlienController::OnToggleMouse);
}

void ACuteAlienController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (PreviewTent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Line Tracing!"));
		// Line Trace from ViewPoint
		FHitResult HitRes;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		FVector Start, End;
		FRotator CamRot;
		float Dist = 1000.f;
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
		
		FTransform HitPointTransform(HitRes.ImpactNormal.Rotation(), HitRes.ImpactPoint, FVector::OneVector);
		PreviewTent->SetActorTransform(HitPointTransform);
	}
}

void ACuteAlienController::OnCancel()
{
	CancelRadialUI();
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
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetIgnoreLookInput(true);
	SetShowMouseCursor(true);
	SetInputMode(InputMode);
}

void ACuteAlienController::ShowRadialUI()
{
	SetIgnoreLookInput(true);

	RadialUI->SetVisibility(ESlateVisibility::Visible);
	bIsRadialVisible = true;
}

void ACuteAlienController::HideRadialUI()
{
	if (RadialUI->GetVisibility() == ESlateVisibility::Hidden)
		return;

	SetIgnoreLookInput(false);

	//////////
	ACuteAlienPlayer* CurPlayer = Cast<ACuteAlienPlayer>(GetPawn());
	CurPlayer->PlayAlienDance();
	//////////

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
	UE_LOG(LogTemp, Warning, TEXT("ShowPreviewTent"));
	// Spawn Preview Tent
	PreviewTent = GetWorld()->SpawnActor<APreviewTentActor>(
		APreviewTentActor::StaticClass(),
		GetPawn()->GetActorLocation(),
		FRotator(0, 0, 0)
	);


	// setactorlocation previewactor tick에서
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

				// [중요] UniqueId가 유효한지 먼저 체크
				if (!OtherPS->GetUniqueId().IsValid()) 
				{
					// ID가 아직 없으면 다음 틱이나 나중에 다시 시도하게 둠 (여기선 로그만)
					UE_LOG(LogTemp, Warning, TEXT("Player ID invalid yet: %s"), *OtherPS->GetPlayerName());
					continue;
				}

				// [핵심 변경] 즉시 실행하지 않고 타이머로 살짝 미룸 (보이스 시스템 동기화 대기)
				FTimerHandle UnusedHandle;
				GetWorld()->GetTimerManager().SetTimer(UnusedHandle, [this, AlienOtherPS, MyChannelID]()
				{
					// 람다 함수 내부에서 다시 유효성 체크 (그 사이 나갔을 수도 있으니)
					if (!IsValid(this) || !IsValid(AlienOtherPS)) return;

					if (AlienOtherPS->VoiceChannelID == MyChannelID)
					{
						// 같은 채널 -> 언뮤트
						GameplayUnmutePlayer(AlienOtherPS->GetUniqueId());
						UE_LOG(LogTemp, Log, TEXT("[Voice] Unmute: %s"), *AlienOtherPS->GetPlayerName());
					}
					else
					{
						// 다른 채널 -> 뮤트
						GameplayMutePlayer(AlienOtherPS->GetUniqueId());
						UE_LOG(LogTemp, Log, TEXT("[Voice] Mute: %s"), *AlienOtherPS->GetPlayerName());
					}
				}, 0.5f, false); // 0.5초 뒤에 실행
			}
		}
	}
}
