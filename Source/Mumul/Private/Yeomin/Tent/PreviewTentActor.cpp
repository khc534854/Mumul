// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Tent/PreviewTentActor.h"

#include "Components/SphereComponent.h"
#include "Yeomin/Tent/TentActor.h"


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
		Comp->bDisallowNanite = true;
		UMaterialInstanceDynamic* DynMat = Comp->CreateAndSetMaterialInstanceDynamic(0);
		SMeshMap.Add(Comp, DynMat);
	}
}

void APreviewTentActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult& SweepResult)
{
	if (OtherActor->IsA(ATentActor::StaticClass()))
	{
		for (TPair<TObjectPtr<UStaticMeshComponent>, TObjectPtr<UMaterialInstanceDynamic>>& Comp : SMeshMap)
		{
			Comp.Value->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(1, 0, 0));
		}
	}
}

void APreviewTentActor::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA(ATentActor::StaticClass()))
	{
		for (TPair<TObjectPtr<UStaticMeshComponent>, TObjectPtr<UMaterialInstanceDynamic>>& Comp : SMeshMap)
		{
			if (Comp.Key->GetName() == TEXT("Cylinder"))
			{
				Comp.Value->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0, 1, 0));
				continue;
			}
			Comp.Value->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(1, 1, 1));
		}
	}
}

// Called every frame
void APreviewTentActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}
