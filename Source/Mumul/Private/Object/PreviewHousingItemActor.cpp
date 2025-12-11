// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/PreviewHousingItemActor.h"

#include "Components/BoxComponent.h"
#include "Object/HousingItemActor.h"
#include "Object/Tent/TentActor.h"


// Sets default values
APreviewHousingItemActor::APreviewHousingItemActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// 충돌 박스 (Trigger)
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;
	BoxComp->SetCollisionProfileName(TEXT("Trigger")); // OverlapOnlyPawn 등 적절한 프로필 사용

	// 메쉬
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
}

// Called when the game starts or when spawned
void APreviewHousingItemActor::BeginPlay()
{
	Super::BeginPlay();
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &APreviewHousingItemActor::OnOverlapBegin);
	BoxComp->OnComponentEndOverlap.AddDynamic(this, &APreviewHousingItemActor::OnOverlapEnd);
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

		// 1. 박스 콜리전 크기를 메쉬에 맞춤
		FVector Center, Extents;
		NewMesh->GetBoundingBox().GetCenterAndExtents(Center, Extents);
		BoxComp->SetBoxExtent(Extents);
		MeshComp->SetRelativeLocation(-Center); // 박스 중심으로 이동

		// 2. 프리뷰 전용 머티리얼 적용 (반투명 효과)
		if (PreviewMaterialBase)
		{
			PreviewDMI = UMaterialInstanceDynamic::Create(PreviewMaterialBase, this);
			
			// 메쉬의 모든 머티리얼 슬롯을 프리뷰 재질로 덮어씌움
			int32 NumMaterials = MeshComp->GetNumMaterials();
			for (int32 i = 0; i < NumMaterials; i++)
			{
				MeshComp->SetMaterial(i, PreviewDMI);
			}
		}

		// 초기 상태 색상 적용
		UpdatePreviewColor();
	}
}

void APreviewHousingItemActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신이나 다른 프리뷰는 무시
	if (OtherActor == this || OtherActor->IsA(APreviewHousingItemActor::StaticClass())) return;

	// 설치 불가능한 대상인지 확인 (태그 또는 클래스)
	// 예: 다른 텐트, 다른 하우징 아이템, 플레이어 등
	if (OtherActor->ActorHasTag(TEXT("MainArea")) || 
		OtherActor->IsA(ATentActor::StaticClass()) || 
		OtherActor->IsA(AHousingItemActor::StaticClass()))
	{
		OverlapCount++;
		UpdatePreviewColor();
	}
}

void APreviewHousingItemActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == this) return;

	if (OtherActor->ActorHasTag(TEXT("MainArea")) || 
		OtherActor->IsA(ATentActor::StaticClass()) || 
		OtherActor->IsA(AHousingItemActor::StaticClass()))
	{
		OverlapCount--;
		if (OverlapCount < 0) OverlapCount = 0;
		
		UpdatePreviewColor();
	}
}

void APreviewHousingItemActor::UpdatePreviewColor()
{
	// 겹친 게 하나라도 있으면 설치 불가
	bIsPlaceable = (OverlapCount == 0);

	if (PreviewDMI)
	{
		FLinearColor Color = bIsPlaceable ? FLinearColor::Green : FLinearColor::Red;
		// 머티리얼에 "EmissiveColor" 또는 "Color" 파라미터가 있어야 함
		PreviewDMI->SetVectorParameterValue(TEXT("EmissiveColor"), Color);
		
		// 투명도 조절 (옵션)
		// PreviewDMI->SetScalarParameterValue(TEXT("Opacity"), 0.5f);
	}
}

