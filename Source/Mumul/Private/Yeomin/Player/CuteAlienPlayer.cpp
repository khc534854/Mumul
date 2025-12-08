// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Player/CuteAlienPlayer.h"

#include "Base/MumulGameState.h"
#include "Components/SceneCaptureComponent2D.h"
#include "GameFramework/SpringArmComponent.h"
#include "khc/Player/MumulPlayerState.h"
#include "khc/Player/VoiceChatComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Yeomin/Player/CuteAlienAnim.h"


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

	VoiceComponent = CreateDefaultSubobject<UVoiceChatComponent>(TEXT("VoiceComponent"));

	MinimapSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("MinimapSpringArm"));
	MinimapSpringArm->SetupAttachment(RootComponent);

	MinimapCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCapture"));
	MinimapCapture->SetupAttachment(MinimapSpringArm);


}

// Called when the game starts or when spawned
void ACuteAlienPlayer::BeginPlay()
{
	Super::BeginPlay();

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

// Called every frame
void ACuteAlienPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACuteAlienPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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
