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
	TArray<UStaticMeshComponent*> AllMeshes;
	GetComponents<UStaticMeshComponent>(AllMeshes);

	if (bIsTargeted)
	{
		if (CachedOriginalMaterials.Num() == 0)
		{
			for (UStaticMeshComponent* Comp : AllMeshes)
			{
				for(UMaterialInterface* Mat : Comp->GetMaterials())
				{
					CachedOriginalMaterials.Add(Mat);
				}
			}
		}

		// 2. 모든 컴포넌트의 머티리얼을 빨간색으로 덮어씌움
		if (DeletePreviewMaterial)
		{
			for (UStaticMeshComponent* Comp : AllMeshes)
			{
				int32 NumMaterials = Comp->GetNumMaterials();
				for (int32 i = 0; i < NumMaterials; i++)
				{
					Comp->SetMaterial(i, DeletePreviewMaterial);
				}
			}
		}
	}
	else
	{
		// [빨강 모드 OFF -> 원상복구]
		if (CachedOriginalMaterials.Num() > 0)
		{
			int32 CachedIndex = 0;
			for (UStaticMeshComponent* Comp : AllMeshes)
			{
				int32 NumMaterials = Comp->GetNumMaterials();
				for (int32 i = 0; i < NumMaterials; i++)
				{
					if (CachedOriginalMaterials.IsValidIndex(CachedIndex))
					{
						Comp->SetMaterial(i, CachedOriginalMaterials[CachedIndex]);
						CachedIndex++;
					}
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
