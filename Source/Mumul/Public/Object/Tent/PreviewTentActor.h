// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreviewTentActor.generated.h"

UCLASS()
class MUMUL_API APreviewTentActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APreviewTentActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY()
	TMap<TObjectPtr<UStaticMeshComponent>, TObjectPtr<UMaterialInstanceDynamic>> SMeshMap;
	UPROPERTY()
	TObjectPtr<class USphereComponent> SphereComp;
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:

	int32 OverlapCount = 0;
	bool bIsPlaceable = true;
};
