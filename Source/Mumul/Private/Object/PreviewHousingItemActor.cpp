// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/PreviewHousingItemActor.h"

#include "Components/BoxComponent.h"
#include "Object/HousingItemActor.h"
#include "Object/Tent/TentActor.h"


// Sets default values
APreviewHousingItemActor::APreviewHousingItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
	// 충돌 박스 (Trigger)
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;
	//BoxComp->SetCollisionProfileName(TEXT("Trigger")); // OverlapOnlyPawn 등 적절한 프로필 사용

	// 메쉬
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	//MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
}

// Called when the game starts or when spawned
void APreviewHousingItemActor::BeginPlay()
{
	Super::BeginPlay();
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &APreviewHousingItemActor::OnOverlapBegin);
	BoxComp->OnComponentEndOverlap.AddDynamic(this, &APreviewHousingItemActor::OnOverlapEnd);

	// 2. [추가] 생성 시 이미 겹쳐있는 액터들 검사 (초기 상태 동기화)
	TArray<AActor*> OverlappingActors;
	BoxComp->GetOverlappingActors(OverlappingActors);

	for (AActor* OverlappedActor : OverlappingActors)
	{
		// OnOverlapBegin 로직을 재활용하거나 직접 구현
		// 여기서는 안전하게 직접 구현 (함수 호출 시그니처 맞추기 번거로움)
        
		if (OverlappedActor == this) continue;

		if (ATentActor* Tent = Cast<ATentActor>(OverlappedActor))
		{
			// 주의: SetOwnerInfo가 BeginPlay보다 늦게 호출될 수 있으므로, 
			// MyUserIndex가 아직 -1일 수 있음. 이 경우 일단 카운트만 하고 나중에 OwnerInfo 설정 시 재검사 필요.
			// 하지만 보통 Controller에서 Spawn -> SetOwnerInfo 순서로 즉시 호출하므로,
			// Tick이나 SetOwnerInfo에서 한 번 더 UpdateColor를 호출해주면 됩니다.
            
			if (Tent->OwnerUserIndex == MyUserIndex)
			{
				ValidTentCount++;
				CurrentTargetTent = Tent;
			}
			else
			{
				OverlapCount++;
			}
		}
		else if (OverlappedActor->IsA(AHousingItemActor::StaticClass()) || OverlappedActor->ActorHasTag("PlayerTent"))
		{
			OverlapCount++;
		}
	}
    
	// 초기 색상 업데이트
	UpdatePreviewColor();
}

// Called every frame
void APreviewHousingItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APreviewHousingItemActor::SetPreviewMesh(UStaticMesh* NewMesh)
{
	if (NewMesh)
	{
		MeshComp->SetStaticMesh(NewMesh);

		// 1. 박스 콜리전 크기 및 메쉬 위치 조정 (기존 코드 유지)
		FVector Center, Extents;
		NewMesh->GetBoundingBox().GetCenterAndExtents(Center, Extents);
		BoxComp->SetBoxExtent(Extents);
		MeshComp->SetRelativeLocation(-Center);

		// 2. [핵심] 모든 머티리얼 슬롯을 프리뷰용 DMI로 교체
		if (PreviewMaterialBase)
		{
			// DMI 생성 (한 번만 생성해서 재사용)
			if (!PreviewDMI)
			{
				PreviewDMI = UMaterialInstanceDynamic::Create(PreviewMaterialBase, this);
			}
            
			// 메쉬의 머티리얼 슬롯 개수만큼 루프
			int32 NumMaterials = MeshComp->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; i++)
			{
				MeshComp->SetMaterial(i, PreviewDMI);
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[Preview] PreviewMaterialBase is NOT set in Blueprint!"));
		}

		// 초기 색상 설정
		UpdatePreviewColor();
	}
}

void APreviewHousingItemActor::SetOwnerInfo(int32 InUserIndex)
{
	MyUserIndex = InUserIndex;
    
	// [중요] 이미 겹쳐있던 텐트들의 유효성을 다시 판단하기 위해 초기화 후 재검사
	ValidTentCount = 0;
	OverlapCount = 0;
	CurrentTargetTent = nullptr;

	TArray<AActor*> OverlappingActors;
	BoxComp->GetOverlappingActors(OverlappingActors);

	for (AActor* OverlappedActor : OverlappingActors)
	{
		if (OverlappedActor == this) continue;

		if (ATentActor* Tent = Cast<ATentActor>(OverlappedActor))
		{
			// 이제 MyUserIndex가 올바르게 설정된 상태에서 비교
			if (Tent->OwnerUserIndex == MyUserIndex)
			{
				ValidTentCount++;
				CurrentTargetTent = Tent;
			}
			else
			{
				OverlapCount++;
			}
		}
		else if (OverlappedActor->IsA(AHousingItemActor::StaticClass()) || OverlappedActor->ActorHasTag("PlayerTent"))
		{
			OverlapCount++;
		}
	}
    
	UpdatePreviewColor();
}

void APreviewHousingItemActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == this || OtherActor->IsA(APreviewHousingItemActor::StaticClass())) return;

	// 1. 텐트와 충돌한 경우
	if (ATentActor* Tent = Cast<ATentActor>(OtherActor))
	{
		// 내 텐트라면? -> 설치 가능한 영역으로 인정
		if (Tent->OwnerUserIndex == MyUserIndex)
		{
			ValidTentCount++;
			CurrentTargetTent = Tent; // 설치 대상 텐트로 등록
		}
		else
		{
			// 남의 텐트라면? -> 장애물
			OverlapCount++;
		}
	}
	// 2. 다른 하우징 아이템이나 설치된 텐트(태그)와 충돌한 경우
	else if (OtherActor->IsA(AHousingItemActor::StaticClass()) || OtherActor->ActorHasTag("PlayerTent")) 
	{
		OverlapCount++;
	}
	// 3. 그 외 (MainArea 등 바닥은 무시)
    
	UpdatePreviewColor();
}

void APreviewHousingItemActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == this) return;

	if (ATentActor* Tent = Cast<ATentActor>(OtherActor))
	{
		if (Tent->OwnerUserIndex == MyUserIndex)
		{
			ValidTentCount--;
			if (ValidTentCount <= 0)
			{
				ValidTentCount = 0;
				CurrentTargetTent = nullptr;
			}
		}
		else
		{
			OverlapCount--;
		}
	}
	else if (OtherActor->IsA(AHousingItemActor::StaticClass()) || OtherActor->ActorHasTag("PlayerTent"))
	{
		OverlapCount--;
	}

	if (OverlapCount < 0) OverlapCount = 0;
	UpdatePreviewColor();
}

void APreviewHousingItemActor::UpdatePreviewColor()
{
	// [조건] 장애물이 없고(OverlapCount == 0) && 내 텐트 안에 있어야 함(ValidTentCount > 0)
	bIsPlaceable = (OverlapCount == 0 && ValidTentCount > 0);

	if (PreviewDMI)
	{
		FLinearColor Color = bIsPlaceable ? FLinearColor::Green : FLinearColor::Red;
		PreviewDMI->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
	}
}

