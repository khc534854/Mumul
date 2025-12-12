// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PreviewHousingItemActor.generated.h"

UCLASS()
class MUMUL_API APreviewHousingItemActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APreviewHousingItemActor();
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// 미리보기 메쉬를 업데이트하고 머티리얼을 설정하는 함수
	void SetPreviewMesh(UStaticMesh* NewMesh);

	// 설치 가능 여부
	bool bIsPlaceable = true;

	void SetOwnerInfo(int32 InUserIndex);

	UPROPERTY(BlueprintReadOnly)
	class ATentActor* CurrentTargetTent;

protected:
	// 내 텐트인지 확인하기 위한 ID
	int32 MyUserIndex = -1;

	// 겹쳐진 "내 텐트"의 개수 (1개 이상이어야 설치 가능)
	int32 ValidTentCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UBoxComponent> BoxComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<class UStaticMeshComponent> MeshComp;

	// [필수] 블루프린트에서 할당해야 함 (반투명, EmissiveColor 파라미터가 있는 머티리얼)
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	TObjectPtr<UMaterialInterface> PreviewMaterialBase;

	// 동적 머티리얼 인스턴스 (색상 변경용)
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewDMI;

	// 오버랩 카운터 (중복 충돌 해결용)
	int32 OverlapCount = 0;

	// 충돌 감지 함수
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 상태 업데이트 (색상 변경)
	void UpdatePreviewColor();
};
