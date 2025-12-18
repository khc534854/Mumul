// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/CampFireActor.h"

#include "Components/SphereComponent.h"
#include "Data/AudioManager.h"
#include "Net/UnrealNetwork.h"
#include "Player/CuteAlienController.h"
#include "Player/MumulPlayerState.h"


// Sets default values
ACampFireActor::ACampFireActor()
{
	PrimaryActorTick.bCanEverTick = false; // 틱은 필요 없음

	// 1. 메시 컴포넌트 설정 (루트)
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));

	// 2. 범위 콜리전 설정
	VoiceRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("VoiceRangeSphere"));
	VoiceRangeSphere->SetupAttachment(RootComponent);
	VoiceRangeSphere->SetSphereRadius(300.f); // 반경 300 (3미터)
	VoiceRangeSphere->SetCollisionProfileName(TEXT("Trigger")); // 오버랩 감지용

	// 3. 이벤트 바인딩
	VoiceRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ACampFireActor::OnOverlapBegin);
	VoiceRangeSphere->OnComponentEndOverlap.AddDynamic(this, &ACampFireActor::OnOverlapEnd);
}

// Called when the game starts or when spawned
void ACampFireActor::BeginPlay()
{
	Super::BeginPlay();

	if (USkeletalMeshComponent* SkeletalMeshComp = GetComponentByClass<USkeletalMeshComponent>())
	{
		FireMID = SkeletalMeshComp->CreateDynamicMaterialInstance(0);
	}

	if (FireMID)
	{
		FireMID->SetScalarParameterValue(TEXT("Opacity"), CurrentOpacity);
	}

	if (HasAuthority())
	{
		OverlappingPawns.Reset();
		OverlapCount = 0;
	}
}

void ACampFireActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//DOREPLIFETIME(ACampFireActor, CampfireChannelID);
	DOREPLIFETIME(ACampFireActor, OverlapCount)
}

void ACampFireActor::OnRep_OverlapCount()
{
	UpdateTargetOpacity();
}

void ACampFireActor::UpdateTargetOpacity()
{
	// 한 명이라도 있으면 1.24, 없으면 0
	TargetOpacity = (OverlapCount > 0) ? 1.24f : 0.0f;

	// 애니메이션 타이머 시작(이미 돌고 있으면 그대로 둠)
	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(OpacityTimer))
		{
			World->GetTimerManager().SetTimer(OpacityTimer, this, &ACampFireActor::TickOpacityAnim, 0.02f, true);
		}
	}
}

void ACampFireActor::TickOpacityAnim()
{
	if (!FireMID)
	{
		GetWorld()->GetTimerManager().ClearTimer(OpacityTimer);
		return;
	}

	// 부드럽게 수렴 (속도 조절 가능)
	CurrentOpacity = FMath::FInterpTo(CurrentOpacity, TargetOpacity, 0.02f, 2.9f);
	FireMID->SetScalarParameterValue(TEXT("Opacity"), CurrentOpacity);

	if (FMath::IsNearlyEqual(CurrentOpacity, TargetOpacity, 0.01f))
	{
		CurrentOpacity = TargetOpacity;
		FireMID->SetScalarParameterValue(TEXT("Opacity"), CurrentOpacity);

		GetWorld()->GetTimerManager().ClearTimer(OpacityTimer);
	}
}

void ACampFireActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                    const FHitResult& SweepResult)
{
	// 1. 들어온 액터가 폰(캐릭터)인지 확인
	APawn* EnteringPawn = Cast<APawn>(OtherActor);
	if (!EnteringPawn) return;

	// 스케일/카운트는 서버에서만 관리 (모든 클라에 replicate)
	if (HasAuthority())
	{
		const bool bAdded = !OverlappingPawns.Contains(EnteringPawn);
		OverlappingPawns.Add(EnteringPawn);

		if (bAdded)
		{
			OverlapCount = OverlappingPawns.Num();
			UpdateTargetOpacity();
		}
	}

	if (!EnteringPawn->IsLocallyControlled()) return;

	// 3. PlayerState를 가져와서 채널 변경 요청
	if (AMumulPlayerState* PS = EnteringPawn->GetPlayerState<AMumulPlayerState>())
	{
		PS->bIsNearByCampFire = true;

		if (PS->WaitingChannelID != TEXT("Lobby"))
		{
			PS->Server_SetVoiceChannelID(PS->WaitingChannelID); // 모닥불 채널로 변경
		}

		if (ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController()))
		{
			PC->AudioManager->StartCampFireSound();
		}
	}
}

void ACampFireActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* ExitingPawn = Cast<APawn>(OtherActor);
	if (!ExitingPawn) return;

	// 스케일/카운트는 서버에서만 관리
	if (HasAuthority())
	{
		const bool bRemoved = OverlappingPawns.Remove(ExitingPawn) > 0;
		if (bRemoved)
		{
			OverlapCount = OverlappingPawns.Num();
			UpdateTargetOpacity();
		}
	}

	if (!ExitingPawn->IsLocallyControlled()) return;

	if (AMumulPlayerState* PS = ExitingPawn->GetPlayerState<AMumulPlayerState>())
	{
		// [디버깅 로그] 현재 채널과 변경할 채널 확인
		UE_LOG(LogTemp, Error, TEXT("Exit Campfire! Current Ch: %s -> Change to Lobby"), *PS->VoiceChannelID);

		PS->bIsNearByCampFire = false;
		PS->Server_SetVoiceChannelID(TEXT("Lobby"));

		if (ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController()))
		{
			PC->AudioManager->EndCampFireSound();
		}
	}
}
