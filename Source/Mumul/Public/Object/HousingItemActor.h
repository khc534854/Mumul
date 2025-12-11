// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HousingItemActor.generated.h"

UCLASS()
class MUMUL_API AHousingItemActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AHousingItemActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBoxComponent* CollisionComp;

	// 설치 가능/불가능 색상 변경
	void SetPlacementStatus(bool bCanPlace);

	// 아이템 ID 저장 (나중에 어떤 아이템인지 확인용)
	FName ItemID;
};
