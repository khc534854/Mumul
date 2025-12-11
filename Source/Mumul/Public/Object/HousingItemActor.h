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
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// 아이템의 외형을 담당하는 메쉬
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Housing")
	TObjectPtr<class UStaticMeshComponent> MeshComp;

	// 충돌 및 오버랩 감지용 (선택 사항, 메쉬 자체 충돌을 쓸 수도 있음)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Housing")
	TObjectPtr<class UBoxComponent> CollisionComp;

	// 어떤 아이템인지 식별하기 위한 ID
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Housing")
	FName ItemID;

	// 주인의 UserIndex (텐트와 소유권 확인용)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Housing")
	int32 OwnerUserIndex;

	// 초기화 함수 (스폰 시 호출)
	void InitHousingItem(FName NewItemID, int32 NewOwnerIndex, UStaticMesh* NewMesh);
};
