// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/ChairObject.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Player/CuteAlienController.h"


// Sets default values
AChairObject::AChairObject()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 1. [트리거] 캡슐 컴포넌트: 접근 감지용 (Overlap Only)
	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
    
	// 플레이어가 지나갈 수 있게 하고, 오버랩 이벤트만 발생시킵니다.
	// "Trigger" 프로필은 보통 Pawn을 Overlap하고 나머지는 무시합니다.
	CollisionComp->SetCollisionProfileName(TEXT("Trigger")); 
	CollisionComp->SetCapsuleSize(100.f, 100.f); // 감지 범위 적절히 조절

	// 2. [물리/클릭] 스태틱 메쉬: 실제 외형 및 라인트레이스 타겟 (Block All)
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
    
	// 물리적 충돌을 켜고, 라인트레이스(Visibility 채널)를 막아야 클릭이 됩니다.
	MeshComp->SetCollisionProfileName(TEXT("BlockAll")); 
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 혹시 BlockAll인데 Visibility가 무시로 되어있을 경우를 대비해 명시적 설정
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// 3. UI 위젯
	InteractionUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionUI"));
	InteractionUI->SetupAttachment(RootComponent);
	InteractionUI->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionUI->SetVisibility(false);

	InteractionUI->SetIsReplicated(false); 
    
	InteractionUI->SetVisibility(false);
}

// Called when the game starts or when spawned
void AChairObject::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionComp)
	{
		CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AChairObject::OnOverlapBegin);
		CollisionComp->OnComponentEndOverlap.AddDynamic(this, &AChairObject::OnOverlapEnd);
	}

	if (InteractionUI)
	{
		InteractionUI->SetVisibility(false); // 기본 숨김
	}
	
}

// Called every frame
void AChairObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AChairObject::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (Pawn->IsLocallyControlled())
		{
			if (InteractionUI) InteractionUI->SetVisibility(true);
            
			if (ACuteAlienController* PC = Cast<ACuteAlienController>(Pawn->GetController()))
			{
				PC->bCanInteract = true;
				PC->TargetChair = this;
			}
		}
	}
}

void AChairObject::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (Pawn->IsLocallyControlled())
		{
			if (InteractionUI) InteractionUI->SetVisibility(false);

			if (ACuteAlienController* PC = Cast<ACuteAlienController>(Pawn->GetController()))
			{
				PC->bCanInteract = false;
				PC->TargetChair = nullptr;
			}
		}
	}
}

