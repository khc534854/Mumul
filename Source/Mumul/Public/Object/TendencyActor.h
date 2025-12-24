#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h" // 필수 헤더
#include "TendencyActor.generated.h"


UCLASS()
class MUMUL_API ATendencyActor : public ACharacter
{
	GENERATED_BODY()

public:
	ATendencyActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tendency")
	int32 TendencyIndex = 0;
    
	void UpdateBodyMaterial(int32 TendencyIdx);
    
	UPROPERTY(EditDefaultsOnly)
	TArray<UMaterialInterface*> PlayerBodyMaterials;
    
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UAnimMontage> IdleMontage;

	// [신규] 데이터 테이블 변수 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
	TObjectPtr<UDataTable> DialogueDataTable;

	// [신규] 랜덤 대사 설정 함수
	void SetRandomDialogue();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<class UWidgetComponent> InteractionUI;
    
	// 기존 DialogueText 변수는 이제 동적으로 변하므로 유지하되, 초기값 용도 등으로 씀
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString DialogueText;
    
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};