// Fill out your copyright notice in the Description page of Project Settings.


#include "MumulMumulGameMode.h"

#include "Yeomin/Tent/TentActor.h"

AMumulMumulGameMode::AMumulMumulGameMode()
{
	static ConstructorHelpers::FClassFinder<ATentActor> TentFinder(
		TEXT("/Game/Yeomin/Actors/Tent/BP_Tent.BP_Tent_C"));
	if (TentFinder.Succeeded())
	{
		TentClass = TentFinder.Class;
	}
}

void AMumulMumulGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < PoolSize; i++)
	{
		ATentActor* Tent = GetWorld()->SpawnActor<ATentActor>(TentClass);
		Tent->Deactivate();
		TentPool.Add(Tent);
	}
}

ATentActor* AMumulMumulGameMode::SpawnTent(const FTransform& SpawnTransform)
{
	for (ATentActor* Tent : TentPool)
	{
		if (!Tent->bIsActive)
		{
			Tent->Activate(SpawnTransform);
			return Tent;
		}
	}
	ATentActor* Tent = GetWorld()->SpawnActor<ATentActor>(TentClass);
	Tent->Activate(SpawnTransform);
	TentPool.Add(Tent);
	return Tent;
}
