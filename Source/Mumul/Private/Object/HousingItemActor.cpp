// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/HousingItemActor.h"


// Sets default values
AHousingItemActor::AHousingItemActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AHousingItemActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHousingItemActor::SetPlacementStatus(bool bCanPlace)
{
}

