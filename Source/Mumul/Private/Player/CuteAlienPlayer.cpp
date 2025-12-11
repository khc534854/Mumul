// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CuteAlienPlayer.h"

#include "Base/MumulGameState.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Data/FCustomItemData.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/MumulPlayerState.h"
#include "Player/VoiceChatComponent.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Network/NetworkStructs.h"
#include "Player/CuteAlienAnim.h"

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

	VoiceComponent = CreateDefaultSubobject<UVoiceChatComponent>(TEXT("VoiceComponent"));

	MinimapSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("MinimapSpringArm"));
	MinimapSpringArm->SetupAttachment(RootComponent);

	MinimapCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("MinimapCapture"));
	MinimapCapture->SetupAttachment(MinimapSpringArm);

	CustomMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CosmeticMesh"));
	CustomMeshComponent->SetupAttachment(GetMesh()); // 캐릭터의 스켈레탈 메시에 부착
	CustomMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CustomMeshComponent->SetRelativeScale3D(FVector::OneVector);
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

void ACuteAlienPlayer::LearningQuizTestFunc()
{
	if (UHttpNetworkSubsystem* HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>())
	{
		FLearningQuizRequest QuizRequest;
		QuizRequest.question = FString(TEXT("판다스에서 데이터프레임을 합치는 방법을 알려줘"));
		QuizRequest.grade = FString(TEXT("초급"));
		
		FString URL = FString(TEXT("/learning_quiz/generate"));
		
		HttpSystem->SendJsonRequest(QuizRequest, URL, &UHttpNetworkSubsystem::OnLearningQuizComplete);
	}
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
