// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/Tent/PreviewTentActor.h"

#include "Components/SphereComponent.h"
#include "Object/Tent/TentActor.h"


// Sets default values
APreviewTentActor::APreviewTentActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APreviewTentActor::BeginPlay()
{
	Super::BeginPlay();

	SphereComp = FindComponentByClass<USphereComponent>();
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &APreviewTentActor::OnOverlap);
	SphereComp->OnComponentEndOverlap.AddDynamic(this, &APreviewTentActor::EndOverlap);

	TArray<UStaticMeshComponent*> SMeshComps;
	GetComponents<UStaticMeshComponent>(SMeshComps);
	for (UStaticMeshComponent* Comp : SMeshComps)
	{
		UMaterialInstanceDynamic* DynMat = Comp->CreateAndSetMaterialInstanceDynamic(0);
		SMeshMap.Add(Comp, DynMat);
	}
}

void APreviewTentActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	if (OtherActor && (OtherActor->ActorHasTag("MainArea") ||
		OtherActor->ActorHasTag("PlayerTent")))
	{
		// 1. 카운트 증가
		OverlapCount++;

		// 2. 하나라도 겹쳐 있다면 설치 불가 처리
		if (OverlapCount > 0)
		{
			bIsPlaceable = false;
            
			// 머티리얼 빨간색 변경
			for (auto& Elem : SMeshMap)
			{
				if (Elem.Value)
				{
					Elem.Value->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(1, 0, 0));
				}
			}
		}
	}
	
	
	// if (OtherActor->IsA(ATentActor::StaticClass()))
	// {
	// 	bIsPlaceable = false;
	// 	for (TPair<TObjectPtr<UStaticMeshComponent>, TObjectPtr<UMaterialInstanceDynamic>>& Comp : SMeshMap)
	// 	{
	// 		Comp.Value->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(1, 0, 0));
	// 	}
	// }
}

void APreviewTentActor::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && (OtherActor->ActorHasTag("MainArea") || OtherActor->ActorHasTag("PlayerTent")))
	{
		// 1. 카운트 감소
		OverlapCount--;

		// 방어 코드: 0보다 작아지지 않게 (혹시 모를 버그 방지)
		if (OverlapCount < 0) OverlapCount = 0;

		// 2. 겹친 물체가 하나도 없을 때만 설치 가능으로 변경
		if (OverlapCount == 0)
		{
			bIsPlaceable = true;

			// 머티리얼 원래 색(초록색)으로 복구
			for (auto& Elem : SMeshMap)
			{
				if (Elem.Value)
				{
					// 특정 파츠(Cylinder)만 다른 색이면 분기 처리
					if (Elem.Key->GetName() == TEXT("Cylinder"))
					{
						Elem.Value->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0, 1, 0));
					}
					else
					{
						Elem.Value->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(1, 1, 1));
					}
				}
			}
		}
	}
}

// Called every frame
void APreviewTentActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}
