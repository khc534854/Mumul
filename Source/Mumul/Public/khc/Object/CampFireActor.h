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

public:
	// 모닥불 외형
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* MeshComp;

	// 보이스 채팅 범위 (콜리전)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* VoiceRangeSphere;

	// 이 모닥불의 고유 채널 ID (에디터에서 설정 가능)
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, Category = "Voice")
	int32 CampfireChannelID = 1;

	// 오버랩 시작 처리
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 오버랩 종료 처리
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};