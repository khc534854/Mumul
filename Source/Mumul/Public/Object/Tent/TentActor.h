// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TentActor.generated.h"

USTRUCT(BlueprintType)
struct FHousingSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FName ItemID;

	UPROPERTY()
	FTransform RelativeTransform;
};

UCLASS()
class MUMUL_API ATentActor : public AActor
{
	GENERATED_BODY()
	ATentActor();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	int32 OwnerUserIndex;
	
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetOwnerUserIndex(const int32 UserIndex);
	UPROPERTY(Replicated)
	bool bIsActive = false;
	void Activate(const FTransform& SpawnTransform);
	void ChangeTransform(const FTransform& SpawnTransform);
	void Deactivate();
	
	float WobbleTime = 0.f;
	float TentSequence1st;
	float TentSequence2nd;
	UFUNCTION(NetMulticast, Reliable)
	void Mulicast_OnScaleAnimation();

	UPROPERTY()
	class ACampFireActor* ChildCampFire;

	UPROPERTY(ReplicatedUsing=OnRep_HousingItems)
	TArray<FHousingSaveData> HousingItems;

	UFUNCTION()
	void OnRep_HousingItems();

	// 아이템 설치 요청 (서버)
	void Server_PlaceHousingItem(FName ItemID, FTransform Transform);

	// 아이템 스폰 및 배치 (클라이언트/서버)
	void SpawnHousingItem(const FHousingSaveData& Data);
};
