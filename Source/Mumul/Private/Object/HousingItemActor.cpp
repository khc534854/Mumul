// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/HousingItemActor.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "Data/FHousingItemData.h"

static const FString HousingItemDataTablePath = TEXT("/Game/Khc/Blueprint/Object/HousingItemList.HousingItemList");

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
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
            TEXT("/Game/Khc/Mat/M_HousingDelete.M_HousingDelete")); 
    if (MatFinder.Succeeded())
    {
        DeletePreviewMaterial = MatFinder.Object;
    }
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

		// [중요] 충돌 설정 재확인 (클라이언트에서도 확실하게)
		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		// [중요] 메쉬 크기에 맞춰 콜리전 박스 크기 및 위치 정렬
		// 이 로직이 클라이언트에서도 실행되어야 라인트레이스가 정확히 맞습니다.
		FVector MinBounds, MaxBounds;
		NewMesh->GetBoundingBox().GetCenterAndExtents(MinBounds, MaxBounds);

		CollisionComp->SetBoxExtent(MaxBounds);
		MeshComp->SetRelativeLocation(-NewMesh->GetBoundingBox().GetCenter());
	}
}

void AHousingItemActor::SetHighlightState(bool bIsTargeted)
{
	if (!MeshComp) return;

	if (bIsTargeted)
	{
		// 1. [빨강 모드 ON]
        
		// 아직 캐싱된 원본이 없다면 현재 상태를 저장 (최초 1회)
		if (CachedOriginalMaterials.Num() == 0)
		{
			CachedOriginalMaterials = MeshComp->GetMaterials();
		}

		// 빨간색 머티리얼이 있다면, 모든 슬롯을 이걸로 덮어씌움
		if (DeletePreviewMaterial)
		{
			int32 NumMaterials = MeshComp->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; i++)
			{
				MeshComp->SetMaterial(i, DeletePreviewMaterial);
			}
		}
	}
	else
	{
		// 2. [빨강 모드 OFF -> 원상복구]
        
		// 저장해둔 원본 머티리얼이 있다면 다시 복구
		if (CachedOriginalMaterials.Num() > 0)
		{
			for (int32 i = 0; i < CachedOriginalMaterials.Num(); i++)
			{
				// 슬롯 인덱스에 맞춰 원본 머티리얼 할당
				MeshComp->SetMaterial(i, CachedOriginalMaterials[i]);
			}
            
			// 복구 후 캐시 비우기 (다음에 다시 저장하기 위해)
			CachedOriginalMaterials.Empty();
		}
	}
}

void AHousingItemActor::OnRep_ItemID()
{
	if (ItemID.IsNone()) return;

	// 데이터 테이블 로드
	UDataTable* HousingTable = LoadObject<UDataTable>(nullptr, *HousingItemDataTablePath);
	if (HousingTable)
	{
		FHousingItemData* ItemData = HousingTable->FindRow<FHousingItemData>(ItemID, TEXT("Housing Client Init"));
		if (ItemData && ItemData->ItemStaticMesh.LoadSynchronous())
		{
			// 서버와 똑같은 초기화 로직 실행 (메쉬 세팅, 위치 정렬, 박스 크기 조절)
			// OwnerIndex는 아직 안 왔을 수도 있지만, 외형 결정에는 ItemID와 Mesh만 있으면 됨
			InitHousingItem(ItemID, OwnerUserIndex, ItemData->ItemStaticMesh.Get());
		}
	}
}
