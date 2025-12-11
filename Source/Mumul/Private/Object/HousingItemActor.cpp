// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/HousingItemActor.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"

AHousingItemActor::AHousingItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // 서버에서 생성되어 클라이언트로 복제됨

	// 충돌 박스 루트 설정
	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;
	CollisionComp->SetCollisionProfileName(TEXT("BlockAll")); // 설치 후에는 길막음

	// 메쉬 설정
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision")); // 충돌은 박스가 담당
}

// Called when the game starts or when spawned
void AHousingItemActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHousingItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHousingItemActor, ItemID);
	DOREPLIFETIME(AHousingItemActor, OwnerUserIndex);
}

void AHousingItemActor::InitHousingItem(FName NewItemID, int32 NewOwnerIndex, UStaticMesh* NewMesh)
{
	ItemID = NewItemID;
	OwnerUserIndex = NewOwnerIndex;

	if (NewMesh)
	{
		MeshComp->SetStaticMesh(NewMesh);
		
		// 메쉬 크기에 맞춰 콜리전 박스 크기 자동 조절
		FVector MinBounds, MaxBounds;
		NewMesh->GetBoundingBox().GetCenterAndExtents(MinBounds, MaxBounds);
		CollisionComp->SetBoxExtent(MaxBounds);
		
		// 메쉬 위치 정렬 (박스 중앙에 오도록)
		// 필요에 따라 오프셋 조정 가능
		MeshComp->SetRelativeLocation(-NewMesh->GetBoundingBox().GetCenter());
	}
}
