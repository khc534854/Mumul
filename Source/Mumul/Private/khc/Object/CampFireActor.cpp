// Fill out your copyright notice in the Description page of Project Settings.


#include "khc/Object/CampFireActor.h"

#include "Components/SphereComponent.h"
#include "khc/Player/MumulPlayerState.h"


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
	
}

void ACampFireActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 1. 들어온 액터가 폰(캐릭터)인지 확인
	APawn* EnteringPawn = Cast<APawn>(OtherActor);
	if (!EnteringPawn) return;

	// 2. [핵심] 들어온 캐릭터가 "나 자신(Local Player)"인지 확인
	// 서버나 다른 클라이언트에서도 이 이벤트가 발생하지만, 
	// 채널 변경 요청(RPC)은 오직 로컬 클라이언트가 자기 PlayerState에게 해야 합니다.
	if (!EnteringPawn->IsLocallyControlled()) return;

	// 3. PlayerState를 가져와서 채널 변경 요청
	if (AMumulPlayerState* PS = EnteringPawn->GetPlayerState<AMumulPlayerState>())
	{
		UE_LOG(LogTemp, Log, TEXT("Enter Campfire Zone! Change Channel to %d"), CampfireChannelID);
		PS->Server_SetVoiceChannelID(CampfireChannelID); // 모닥불 채널로 변경
	}
}

void ACampFireActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* ExitingPawn = Cast<APawn>(OtherActor);
	if (!ExitingPawn) return;

	if (!ExitingPawn->IsLocallyControlled()) return;

	if (AMumulPlayerState* PS = ExitingPawn->GetPlayerState<AMumulPlayerState>())
	{
		// [디버깅 로그] 현재 채널과 변경할 채널 확인
		UE_LOG(LogTemp, Error, TEXT("Exit Campfire! Current Ch: %d -> Change to 0"), PS->VoiceChannelID);
       
		PS->Server_SetVoiceChannelID(0); 
	}
}
