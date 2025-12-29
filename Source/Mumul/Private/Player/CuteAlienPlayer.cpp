// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CuteAlienPlayer.h"

#include "EnhancedInputComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Base/MumulGameState.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/WidgetSwitcher.h"
#include "Data/FCustomItemData.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/MumulPlayerState.h"
#include "Player/VoiceChatComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Object/OXQuizTriggerActor.h"
#include "Player/CuteAlienAnim.h"
#include "Player/CuteAlienController.h"
#include "Player/Component/PlayerOXQuizComponent.h"
#include "UI/OXQuiz/AskOXQuizUI.h"
#include "UI/OXQuiz/OXQuizUI.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Image.h"
#include "Data/AudioManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/VoiceConfig.h"
#include "Object/CampFireActor.h"

static const FString ItemDataTablePath = TEXT("/Game/Khc/Blueprint/Object/CustomItemList.CustomItemList");
// Sets default values
ACuteAlienPlayer::ACuteAlienPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> Dance1MontageFinder(
		TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Animation2/No_Montage.No_Montage"));
	if (Dance1MontageFinder.Succeeded())
	{
		DanceMontage1 = Dance1MontageFinder.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Dance2MontageFinder(
		TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Animation2/PopPinDance_Montage.PopPinDance_Montage"));
	if (Dance2MontageFinder.Succeeded())
	{
		DanceMontage2 = Dance2MontageFinder.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Dance3MontageFinder(
		TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Animation2/StepDance_Montage.StepDance_Montage"));
	if (Dance3MontageFinder.Succeeded())
	{
		DanceMontage3 = Dance3MontageFinder.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Dance4MontageFinder(
		TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Animation2/FootDance_Montage.FootDance_Montage"));
	if (Dance1MontageFinder.Succeeded())
	{
		DanceMontage4 = Dance4MontageFinder.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Dance5MontageFinder(
		TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Animation2/Greeting0_Montage.Greeting0_Montage"));
	if (Dance2MontageFinder.Succeeded())
	{
		DanceMontage5 = Dance5MontageFinder.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Dance6MontageFinder(
		TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Animation2/Greeting1_Montage.Greeting1_Montage"));
	if (Dance3MontageFinder.Succeeded())
	{
		DanceMontage6 = Dance6MontageFinder.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> Dance7MontageFinder(
		TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Animation2/Greeting2_Montage.Greeting2_Montage"));
	if (Dance3MontageFinder.Succeeded())
	{
		DanceMontage7 = Dance7MontageFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimMontage> SitMontageFinder(
	TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Animation2/Sitting_Montage.Sitting_Montage"));
	if (SitMontageFinder.Succeeded())
	{
		SitMontage = SitMontageFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_ClickFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_Click.IA_Click"));
	if (IA_ClickFinder.Succeeded())
	{
		IA_Click = IA_ClickFinder.Object;
	}

	UIInteractionComp = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("UI InteractionComp"));
	UIInteractionComp->SetupAttachment(GetFollowCamera());
	UIInteractionComp->InteractionDistance = 1200.f;

	VoiceComponent = CreateDefaultSubobject<UVoiceChatComponent>(TEXT("VoiceComponent"));

	MinimapSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("MinimapSpringArm"));
	MinimapSpringArm->SetupAttachment(RootComponent);

	MinimapCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCapture"));
	MinimapCapture->SetupAttachment(MinimapSpringArm);

	CustomMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CosmeticMesh"));
	CustomMeshComponent->SetupAttachment(GetMesh()); // 캐릭터의 스켈레탈 메시에 부착
	CustomMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CustomMeshComponent->SetRelativeScale3D(FVector::OneVector);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameTagComponent"));
	WidgetComponent->SetupAttachment(GetRootComponent());

	WidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f)); // 캐릭터 키에 맞춰 Z값 조절 필요
	WidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	WidgetComponent->SetCullDistance(1500.0f);
	WidgetComponent->SetDrawAtDesiredSize(true);
}

// Called when the game starts or when spawned
void ACuteAlienPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocallyControlled())
	{
		UIInteractionComp->Deactivate();
		UIInteractionComp->SetComponentTickEnabled(false);
	}

	if (IsLocallyControlled())
	{
		if (MinimapCapture)
		{
			// 2. 렌더 타겟을 동적으로 생성 (너비, 높이는 256, 512 등 원하는 해상도로)
			MinimapRenderTarget = UKismetRenderingLibrary::CreateRenderTarget2D(this, 512, 512);

			// 3. 생성된 렌더 타겟을 캡처 컴포넌트에 연결
			MinimapCapture->TextureTarget = MinimapRenderTarget;

			// 4. 캡처 시작
			MinimapCapture->CaptureScene(); // 혹은 CaptureEveryFrame이 켜져있다면 자동 시작됨
		}
		WidgetComponent->SetVisibility(false);
	}
	else
	{
		// 내 캐릭터가 아니면 캡처 컴포넌트를 꺼서 성능 낭비를 막습니다.
		if (MinimapCapture)
		{
			MinimapCapture->Deactivate();
			MinimapCapture->SetComponentTickEnabled(false);
		}
	}

	if (AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>())
	{
		UpdateBodyMaterial(PS->PS_TendencyID);
		// 이미 장착된 아이템이 있다면 적용 (Replication 타이밍 이슈 방지)
		UpdateCustomMesh(PS->EquippedCustomID);
		UpdateNameTag();
	}

	if (WidgetComponent)
	{
		WidgetComponent->SetVisibility(false);
	}
}

void ACuteAlienPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && GetPlayerState())
	{
		AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();
		if (PS && PS->PS_UserIndex > 0)
		{
			if (AMumulGameState* GS = GetWorld()->GetGameState<AMumulGameState>())
			{
				// 소멸 직전의 현재 위치를 저장
				GS->Multicast_SavePlayerLocation(PS->PS_UserIndex, GetActorTransform());
				UE_LOG(LogTemp, Warning, TEXT("[Player] Saved Location on EndPlay: User %d"), PS->PS_UserIndex);
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ACuteAlienPlayer::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>())
	{
		UpdateBodyMaterial(PS->PS_TendencyID);
		UpdateCustomMesh(PS->EquippedCustomID);
		UpdateNameTag();
	}
}

// Called every frame
void ACuteAlienPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateVoiceIconState();
	UpdateNameTagVisibility();
}

// Called to bind functionality to input
void ACuteAlienPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	Input->BindAction(IA_Click, ETriggerEvent::Started, this, &ACuteAlienPlayer::OnClickInteraction);
}

void ACuteAlienPlayer::OnClickInteraction()
{
	const FHitResult& Hit = UIInteractionComp->GetLastHitResult();
	if (Hit.GetComponent())
	{
		UWidgetComponent* WidgetComp = Cast<UWidgetComponent>(Hit.GetComponent());
		if (WidgetComp)
		{
			AOXQuizTriggerActor* QuizTriggerActor = Cast<AOXQuizTriggerActor>(WidgetComp->GetOwner());
			ACuteAlienController* PC = Cast<ACuteAlienController>(GetController());
			PC->AudioManager->PlayClickSound();
			
			if (PC->OXQuizComp->OXQuizUI->GetVisibility() == ESlateVisibility::Collapsed || PC->OXQuizComp->OXQuizUI->GetVisibility() == ESlateVisibility::SelfHitTestInvisible)
			{
				// Set Mouse
				int32 SizeX, SizeY;
				PC->GetViewportSize(SizeX, SizeY);
				PC->SetMouseLocation(SizeX / 2, SizeY / 2);
				PC->OnCancelUI();
				FInputModeGameAndUI InputMode;
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
				PC->SetIgnoreLookInput(true);
				PC->SetShowMouseCursor(true);
				PC->SetInputMode(InputMode);

				// Set OXQuiz
				PC->OXQuizComp->OXQuizUI->AskOXQuizUI->SetQuizTriggerActor(QuizTriggerActor);
				PC->OXQuizComp->OXQuizUI->AskOXQuizUI->SetPlayerController(PC);
				PC->OXQuizComp->OXQuizUI->OXQuizWS->SetActiveWidgetIndex(2);
				PC->OXQuizComp->OXQuizUI->AskOXQuizUI->SetAskQuizText(QuizTriggerActor->GetDifficultyText());
				PC->OXQuizComp->OXQuizUI->PlayAnimation(PC->OXQuizComp->OXQuizUI->Confirm_PopUp);
			}
			PC->OXQuizComp->OXQuizUI->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void ACuteAlienPlayer::UpdateVoiceIconState()
{
	// 1. 이름표 위젯 가져오기
	if (!WidgetComponent) return;
	UUserWidget* WidgetObj = WidgetComponent->GetUserWidgetObject();
	if (!WidgetObj) return;

	// 2. 말하기 아이콘 이미지 가져오기 (이름은 위젯 에디터에서 정한 것과 같아야 함)
	UImage* MicIcon = Cast<UImage>(WidgetObj->GetWidgetFromName(TEXT("SpeakingIcon")));
	if (!MicIcon) return;

	// 3. PlayerState 확인
	AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();
	if (!PS) return;

	// 4. 이 플레이어의 VOIP Talker 가져오기 (엔진이 관리하는 객체)
	// GetUniqueId()를 통해 나든 남이든 상관없이 Talker를 찾습니다.
	UVOIPTalker* Talker = UVOIPStatics::GetVOIPTalkerForPlayer(PS->GetUniqueId());

	bool bIsTalking = false;
	if (Talker)
	{
		// 5. 목소리 레벨 확인 (0.01f 이상이면 말하는 중으로 간주)
		if (Talker->GetVoiceLevel() > 0.01f)
		{
			bIsTalking = true;
		}
	}

	// 6. 상태에 따라 아이콘 켜고 끄기
	if (bIsTalking)
	{
		if (MicIcon->GetVisibility() != ESlateVisibility::Visible)
		{
			MicIcon->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		if (MicIcon->GetVisibility() != ESlateVisibility::Hidden)
		{
			MicIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ACuteAlienPlayer::UpdateBodyMaterial(int32 TendencyIdx)
{
	if (PlayerBodyMaterials.Num() == 0) return;

	int32 MatIndexStart = 0;

	if (TendencyIdx <= 1)
	{
		MatIndexStart = 0;
	}
	else if (TendencyIdx >= 2 && TendencyIdx <= 5)
	{
		MatIndexStart = (TendencyIdx - 1) * 3;
	}

	if (PlayerBodyMaterials.IsValidIndex(MatIndexStart + 2))
	{
		GetMesh()->SetMaterial(0, PlayerBodyMaterials[MatIndexStart]);
		GetMesh()->SetMaterial(2, PlayerBodyMaterials[MatIndexStart + 1]);
		GetMesh()->SetMaterial(3, PlayerBodyMaterials[MatIndexStart + 2]);
	}
}

void ACuteAlienPlayer::Server_EquipCustom_Implementation(FName ItemID)
{
	AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();
	if (PS)
	{
		// 이미 장착된 아이템을 해제하거나, 새 아이템을 장착합니다.
		if (PS->EquippedCustomID == ItemID)
		{
			// 같은 아이템을 다시 클릭하면 해제합니다.
			PS->EquippedCustomID = NAME_None;
		}
		else
		{
			PS->EquippedCustomID = ItemID;
		}

		PS->OnRep_EquippedCustomID();

		if (AMumulGameState* GS = GetWorld()->GetGameState<AMumulGameState>())
		{
			GS->Multicast_SavePlayerCosmetic(PS->PS_UserIndex, PS->EquippedCustomID);
		}
	}
}

void ACuteAlienPlayer::Server_SitDown_Implementation()
{
	Multicast_SitDown();
}

void ACuteAlienPlayer::Multicast_SitDown_Implementation()
{
	PlayerAnim->Montage_Play(SitMontage);
}

void ACuteAlienPlayer::UpdateNameTag()
{
	AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();
	if (!PS)
	{
		// 재시도 타이머
		FTimerHandle WaitHandle;
		GetWorldTimerManager().SetTimer(WaitHandle, this, &ACuteAlienPlayer::UpdateNameTag, 0.5f, false);
		return;
	}

	// 1. 텍스트 내용만 업데이트 (가시성 로직 삭제)
	UUserWidget* WidgetObj = WidgetComponent->GetUserWidgetObject();
	if (WidgetObj)
	{
		if (UTextBlock* TextBlock = Cast<UTextBlock>(WidgetObj->GetWidgetFromName(TEXT("NameText"))))
		{
			FString DisplayName = PS->PS_RealName;
			TextBlock->SetText(FText::FromString(DisplayName));
		}
	}
}

void ACuteAlienPlayer::UpdateCustomMesh(FName ItemID)
{
	if (!CustomMeshComponent) return;

	if (ItemID == NAME_None) // 아이템 해제 명령
	{
		CustomMeshComponent->SetStaticMesh(nullptr);
		// 부착 상태를 유지할 경우 부착 해제는 선택적입니다. (KeepRelativeTransform)
		// CosmeticMeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		return;
	}

	// 1. 데이터 테이블 로드 및 아이템 데이터 찾기 (동기 로드)
	UDataTable* ItemDataTable = LoadObject<UDataTable>(nullptr, *ItemDataTablePath);

	if (ItemDataTable)
	{
		// FCustomItemData를 사용
		FCustomItemData* ItemData = ItemDataTable->FindRow<FCustomItemData>(ItemID, TEXT("Cosmetic Load"));

		if (ItemData)
		{
			// TSoftObjectPtr의 에셋을 동기적으로 로드
			UStaticMesh* MeshToEquip = ItemData->ItemStaticMesh.LoadSynchronous();

			if (MeshToEquip)
			{
				// 2. 메시 설정 및 부착
				CustomMeshComponent->SetStaticMesh(MeshToEquip);

				// 소켓에 부착 (GetMesh()는 캐릭터의 스켈레탈 메시 컴포넌트)
				CustomMeshComponent->AttachToComponent(
					GetMesh(),
					FAttachmentTransformRules::SnapToTargetNotIncludingScale,
					ItemData->AttachSocketName // 데이터에 저장된 소켓 이름 사용
				);

				// 3. 트랜스폼 오프셋 적용
				CustomMeshComponent->SetRelativeTransform(ItemData->RelativeTransform);
				return;
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[Cosmetic] Failed to load Static Mesh for item: %s"), *ItemID.ToString());
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Cosmetic] Failed to find row for item: %s"), *ItemID.ToString());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[Cosmetic] Item DataTable not found at: %s"), *ItemDataTablePath);
	}

	// 실패 시 안전하게 메시 제거
	CustomMeshComponent->SetStaticMesh(nullptr);
}

void ACuteAlienPlayer::Server_PlayAlienDance_Implementation(int32 SelectIdx)
{
	Multicast_PlayAlienDance(SelectIdx);
}

void ACuteAlienPlayer::Multicast_PlayAlienDance_Implementation(int32 SelectIdx)
{
	// [추가] 인덱스에 따른 행동 분기
	switch (SelectIdx)
	{
	case 0: // 슬롯 0번 (예: 춤추기)
		{
			if (PlayerAnim->Montage_IsPlaying(DanceMontage1))
				return;

			PlayerAnim->Montage_Play(DanceMontage1);
			break;
		}
	case 1: // 슬롯 1번 (예: 인사하기)
		{
			if (PlayerAnim->Montage_IsPlaying(DanceMontage2))
				return;

			PlayerAnim->Montage_Play(DanceMontage2);
			break;
		}
	case 2: // 슬롯 2번 (예: 앉기)
		{
			if (PlayerAnim->Montage_IsPlaying(DanceMontage3))
				return;

			PlayerAnim->Montage_Play(DanceMontage3);
			break;
		}
	case 3: // 슬롯 0번 (예: 춤추기)
		{
			if (PlayerAnim->Montage_IsPlaying(DanceMontage4))
				return;

			PlayerAnim->Montage_Play(DanceMontage4);
			break;
		}
	case 4: // 슬롯 1번 (예: 인사하기)
		{
			if (PlayerAnim->Montage_IsPlaying(DanceMontage5))
				return;

			PlayerAnim->Montage_Play(DanceMontage5);
			break;
		}
	case 5: // 슬롯 2번 (예: 앉기)
		{
			if (PlayerAnim->Montage_IsPlaying(DanceMontage6))
				return;

			PlayerAnim->Montage_Play(DanceMontage6);
			break;
		}
	case 6: // 슬롯 2번 (예: 앉기)
		{
			if (PlayerAnim->Montage_IsPlaying(DanceMontage7))
				return;

			PlayerAnim->Montage_Play(DanceMontage7);
			break;
		}
	default:
		break;
	}
}


void ACuteAlienPlayer::Server_PlayElectrocutedMontage_Implementation(FVector FireLocation,
                                                                     FVector FireDirection)
{
	Multicast_PlayElectrocutedMontage(FireLocation, FireDirection);
}


void ACuteAlienPlayer::Multicast_PlayElectrocutedMontage_Implementation(FVector FireLocation,
                                                                        FVector FireDirection)
{
	if (!ElectrocutedMontage)
		return;

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
		return;

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
		return;

	if (AnimInstance->Montage_IsPlaying(ElectrocutedMontage))
		return;

	if (!LightningBoltVFX)
		return;;

	// ✔ 스폰 로테이션: 위를 향하되 Yaw만 캐릭터 기준
	FRotator SpawnRotation = GetActorRotation();
	SpawnRotation.Pitch = -90.f;
	SpawnRotation.Roll = 0.f;

	// ✔ Niagara 스폰 (위치는 의미 없음, 기준점만 제공)
	UNiagaraComponent* BoltNiagaraComp =
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			LightningBoltVFX,
			FireLocation,
			SpawnRotation,
			FVector(0.5f),
			true,
			false
		);

	if (BoltNiagaraComp)
	{
		BoltNiagaraComp->SetVectorParameter(
			FName("LightingVector"),
			FireDirection
		);

		float Dist = FVector::Dist(FireLocation, GetActorLocation());
		BoltNiagaraComp->SetVariableVec2(
			FName("LightingSize"),
			FVector2D(200.f, Dist)
		);

		BoltNiagaraComp->Activate(true);
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		LightningImpactVFX,
		GetActorLocation() + FVector(0.f, 0.f, -104.f),
		FRotator::ZeroRotator,
		FVector(0.5f),
		true,
		true
	);

	AnimInstance->Montage_Play(ElectrocutedMontage, 1.f);

	UGameplayStatics::PlaySoundAtLocation(
		this,
		ElectricShock,
		GetActorLocation()
	);
}

void ACuteAlienPlayer::PlayTentSpawnSound()
{
	UAudioComponent* InstallAudioComp =
	UGameplayStatics::SpawnSoundAtLocation(
		this,
		Zip,
		GetActorLocation()
	);

	if (InstallAudioComp)
	{
		InstallAudioComp->OnAudioFinished.AddDynamic(this, &ACuteAlienPlayer::OnSoundFinished);
	}
}

void ACuteAlienPlayer::SetIsMeetingSitting(bool bIsSitting, AActor* FocusTarget)
{
    if (!GetCameraBoom())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Player] No Camera Boom"));
        return;
    }

    if (bIsSitting)
    {
        // 1. 물리/이동 멈춤
        GetCharacterMovement()->DisableMovement();
        GetCharacterMovement()->StopMovementImmediately();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

        // [신규 1] 캐릭터 회전 및 컨트롤러 시선 강제 고정 (모닥불 바라보기)
        if (FocusTarget)
        {
            FVector FireLoc = FocusTarget->GetActorLocation();
            FVector MyLoc = GetActorLocation();
            FVector DirToFire = (FireLoc - MyLoc).GetSafeNormal2D();
            FRotator LookAtFireRot = DirToFire.Rotation();

            // (A) 캐릭터 몸 회전
            SetActorRotation(LookAtFireRot);

            // (B) 컨트롤러 시선 회전 (중요: 이걸 해야 카메라가 홱 돌아가지 않음)
            if (IsLocallyControlled())
            {
                if (AController* PC = GetController())
                {
                    PC->SetControlRotation(LookAtFireRot);
                }
            }
        }

        Server_SitDown();

        if (FocusTarget)
        {
            OriginalCameraParent = GetCameraBoom()->GetAttachParent();
            OriginalCameraTransform = GetCameraBoom()->GetRelativeTransform();
            OriginalArmLength = GetCameraBoom()->TargetArmLength;
            OriginalSocketOffset = GetCameraBoom()->SocketOffset;

            FVector FireCenter = FocusTarget->GetActorLocation() + FVector(0.f, 0.f, 120.f);

            GetCameraBoom()->AttachToComponent(FocusTarget->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
            GetCameraBoom()->SetWorldLocation(FireCenter);

            GetCameraBoom()->TargetArmLength = 0.0f; // 중심점 뷰
            GetCameraBoom()->SocketOffset = FVector::ZeroVector;
            
            if (IsLocallyControlled())
            {
                if (APlayerController* PC = Cast<APlayerController>(GetController()))
                {
                	FVector DirToMe = (GetActorLocation() - FireCenter).GetSafeNormal();
                	FRotator LookAtRot = DirToMe.Rotation();

                	PC->SetShowMouseCursor(false); 
                	FInputModeGameOnly InputMode;
                	PC->SetInputMode(InputMode);
                	PC->FlushPressedKeys(); // 눌려있는 키 상태 초기화

                	PC->SetControlRotation(LookAtRot);

                	bool bWasLagEnabled = GetCameraBoom()->bEnableCameraLag;
                	GetCameraBoom()->bEnableCameraLag = false; 
                    
                	if (PC->PlayerCameraManager)
                	{
                		PC->PlayerCameraManager->UpdateCamera(0.0f);
                	}

                	GetCameraBoom()->bEnableCameraLag = bWasLagEnabled;
                }
            }
        }
    }
    else
    {
        // 1. 이동 재개
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    	Server_StandUp();

        // 3. [신규] 카메라 복구 (캐릭터 등 뒤로)
        if (OriginalCameraParent)
        {
            GetCameraBoom()->AttachToComponent(OriginalCameraParent, FAttachmentTransformRules::KeepWorldTransform);
            
            // 원래 위치/설정으로 복원
            GetCameraBoom()->SetRelativeTransform(OriginalCameraTransform);
            GetCameraBoom()->TargetArmLength = OriginalArmLength;
            GetCameraBoom()->SocketOffset = OriginalSocketOffset;
        }
    }
}

void ACuteAlienPlayer::UpdateNameTagVisibility()
{
	if (!WidgetComponent) return;

	// 1. 내 캐릭터(로컬)인 경우 무조건 숨김
	if (IsLocallyControlled())
	{
		if (WidgetComponent->IsVisible()) WidgetComponent->SetVisibility(false);
		return;
	}

	// 2. 관전자(내 화면의 플레이어) 찾기
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	if (!LocalPC) return;

	APawn* LocalPawn = LocalPC->GetPawn();
	if (!LocalPawn) return;

	// 3. 인트로 확인 (방장이면 인트로가 끝났어야 보임)
	if (ACuteAlienController* AlienPC = Cast<ACuteAlienController>(LocalPC))
	{
		if (!AlienPC->bIsIntroFinished)
		{
			if (WidgetComponent->IsVisible()) WidgetComponent->SetVisibility(false);
			return;
		}
	}

	// 4. 거리 계산 (제곱 거리 사용으로 최적화 - Sqrt 연산 생략)
	float DistSq = FVector::DistSquared(GetActorLocation(), LocalPawn->GetActorLocation());
	float VisibleDistSq = NameTagVisibleDistance * NameTagVisibleDistance;

	// 5. 조건에 따라 켜고 끄기
	bool bShouldBeVisible = (DistSq <= VisibleDistSq);

	if (WidgetComponent->IsVisible() != bShouldBeVisible)
	{
		WidgetComponent->SetVisibility(bShouldBeVisible);
	}
}

void ACuteAlienPlayer::Multicast_SitAtLocation_Implementation(FVector TargetLoc, FRotator TargetRot,
                                                              ACampFireActor* TargetFire)
{
	// 1. [텔레포트] 물리 가속도 무시하고 즉시 이동
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}
    
	// TeleportPhysics: 슥 미끄러지는 현상 방지
	SetActorLocationAndRotation(TargetLoc, TargetRot, false, nullptr, ETeleportType::TeleportPhysics);

	// 2. 애니메이션 및 상태 변경 (기존 함수 활용)
	// (TargetFire는 카메라 부착용으로 넘겨줌)
	SetIsMeetingSitting(true, TargetFire); 

	// 3. [핵심] 로컬 컨트롤러(나) 상태 동기화
	// 멀티캐스트는 모든 클라이언트에서 실행되지만, 입력 제한과 변수 저장은 '주인'만 하면 됩니다.
	if (IsLocallyControlled())
	{
		if (ACuteAlienController* MyPC = Cast<ACuteAlienController>(GetController()))
		{
			// (1) 시선 강제 고정
			MyPC->SetControlRotation(TargetRot);

			// (2) 입력 제한
			MyPC->SetIgnoreMoveInput(true);
			MyPC->SetIgnoreLookInput(false); // 마우스는 움직이게

			// (3) 로컬 변수 동기화 (일어날 때 사용)
			MyPC->CurrentMeetingCampFire = TargetFire;
		}
	}
}

void ACuteAlienPlayer::Server_StandUp_Implementation()
{
	Multicast_StandUp();
}

void ACuteAlienPlayer::Multicast_StandUp_Implementation()
{
	if (PlayerAnim)
	{
		// 앉기 루프 정지
		if (PlayerAnim->Montage_IsPlaying(SitMontage))
		{
			PlayerAnim->Montage_Stop(0.5f, SitMontage);
		}
		// 일어서기 몽타주 재생 (모든 클라이언트에서 보임)
		if (StandUpMontage)
		{
			PlayerAnim->Montage_Play(StandUpMontage);
		}
	}
}


void ACuteAlienPlayer::OnSoundFinished()
{
	UGameplayStatics::PlaySound2D(
	this,
	Boing
	);
}
