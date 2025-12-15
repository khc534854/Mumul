// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/HousingItemActor.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "Data/FHousingItemData.h"

static const FString HousingItemDataTablePath = TEXT("/Game/Khc/Blueprint/Object/HousingItemList.HousingItemList");

AHousingItemActor::AHousingItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	//bReplicates = true; // 서버에서 생성되어 클라이언트로 복제됨

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
		// 1. 메쉬 교체 (이때 머티리얼이 초기화됨)
		MeshComp->SetStaticMesh(NewMesh);

		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		MeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
		MeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		FVector MinBounds, MaxBounds;
		NewMesh->GetBoundingBox().GetCenterAndExtents(MinBounds, MaxBounds);
		CollisionComp->SetBoxExtent(MaxBounds);
		MeshComp->SetRelativeLocation(-NewMesh->GetBoundingBox().GetCenter());

		// 2. [핵심 수정] 하이라이트 상태였다면, 메쉬 교체 후 즉시 다시 빨간색 적용
		if (bIsHighlighted)
		{
			// 강제로 다시 칠하기 (true를 넣어 재호출)
			SetHighlightState(true); 
		}
	}
}

void AHousingItemActor::SetHighlightState(bool bIsTargeted)
{
	// [수정] 방어 코드: 메쉬가 없으면 중단
	if (!MeshComp || !MeshComp->GetStaticMesh()) return;

	// 상태 저장 (InitHousingItem에서 복구하기 위함)
	bIsHighlighted = bIsTargeted;

	if (bIsTargeted)
	{
		// [빨강 모드 ON]
        
		// 1. 원본 머티리얼 캐싱 (비어있을 때만)
		if (CachedOriginalMaterials.Num() == 0)
		{
			CachedOriginalMaterials = MeshComp->GetMaterials();
		}

		// 2. [수정] GetComponents 루프 제거 -> MeshComp만 확실하게 처리
		// 다른 더미 컴포넌트가 영향을 주지 않도록 함
		if (DeletePreviewMaterial)
		{
			// 원본 메쉬 에셋의 슬롯 개수만큼 루프
			int32 NumMaterials = MeshComp->GetStaticMesh()->GetStaticMaterials().Num();
          
			for (int32 i = 0; i < NumMaterials; i++)
			{
				MeshComp->SetMaterial(i, DeletePreviewMaterial);
			}
		}
	}
	else
	{
		// [빨강 모드 OFF -> 원상복구]
		if (CachedOriginalMaterials.Num() > 0)
		{
			for (int32 i = 0; i < CachedOriginalMaterials.Num(); i++)
			{
				// 유효성 체크 후 복구
				if (CachedOriginalMaterials.IsValidIndex(i) && CachedOriginalMaterials[i])
				{
					MeshComp->SetMaterial(i, CachedOriginalMaterials[i]);
				}
			}
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
