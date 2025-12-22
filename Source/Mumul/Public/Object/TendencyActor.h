// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TendencyActor.generated.h"

UCLASS()
class MUMUL_API ATendencyActor : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ATendencyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tendency")
	int32 TendencyIndex = 0;
	
	void UpdateBodyMaterial(int32 TendencyIdx);
	
	UPROPERTY(EditDefaultsOnly)
	TArray<UMaterialInterface*> PlayerBodyMaterials;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UAnimMontage> IdleMontage;
	
	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<class UWidgetComponent> InteractionUI;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FString DialogueText;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
