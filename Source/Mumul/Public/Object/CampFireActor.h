// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CampFireActor.generated.h"

UCLASS()
class MUMUL_API ACampFireActor : public AActor
{
	GENERATED_BODY()

public:
	ACampFireActor();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> FireMID;

	UPROPERTY(ReplicatedUsing=OnRep_OverlapCount)
	int32 OverlapCount = 0;
	UFUNCTION()
	void OnRep_OverlapCount();
	void UpdateTargetOpacity();
	UPROPERTY()
	TSet<TObjectPtr<class APawn>> OverlappingPawns;

	float CurrentOpacity = 0.f;
	float TargetOpacity = 0.f;
	
	FTimerHandle OpacityTimer;
	void TickOpacityAnim();

public:
	// 모닥불 외형
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComp;

	// 보이스 채팅 범위 (콜리전)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USphereComponent> VoiceRangeSphere;

	// // 이 모닥불의 고유 채널 ID (에디터에서 설정 가능)
	// UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Voice")
	// int32 CampfireChannelID = 1;

	// 오버랩 시작 처리
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 오버랩 종료 처리
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                  int32 OtherBodyIndex);

	// [신규] 좌석 위치 컴포넌트 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Meeting")
	TArray<TObjectPtr<USceneComponent>> SeatPoints;

	// [신규] 자리 점유 현황 (UserID -> SeatIndex)
	UPROPERTY()
	TMap<APlayerState*, int32> OccupiedSeats;

	// [신규] 빈 자리를 찾아 할당하고 Transform 반환 (Server Only)
	bool AssignAvailableSeat(APlayerState* Requester, FTransform& OutTransform);

	// [신규] 자리를 비움 (Server Only)
	void ReleaseSeat(APlayerState* Requester);
};
