// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TentActor.generated.h"

UCLASS()
class MUMUL_API ATentActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATentActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(Replicated)
	bool bIsActive = false;
	void Activate(const FTransform& SpawnTransform);
	void Deactivate();
};
