// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Tent/TentActor.h"

#include "Net/UnrealNetwork.h"


// Sets default values
ATentActor::ATentActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	AActor::SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ATentActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATentActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATentActor, bIsActive)
}

// Called every frame
void ATentActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FMath::IsNearlyEqual(TentScale, 1.f, 0.1f))
	{
		TentScale -= 0.1f;
		SetActorScale3D(FVector(TentScale));
	}
}

void ATentActor::Activate(const FTransform& SpawnTransform)
{
	SetActorTransform(SpawnTransform);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	bIsActive = true;
}

void ATentActor::ChangeTransform(const FTransform& SpawnTransform)
{
	SetActorTransform(SpawnTransform);
}

void ATentActor::Deactivate()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	bIsActive = false;
}

void ATentActor::Mulicast_OnScaleAnimation_Implementation()
{
	TentScale = 5.f;
}
