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

		if (Tent == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Tent Spawn FAILED at %d"), i);
			continue;
		}
		
		Tent->Deactivate();
		TentPool.Add(Tent);
	}
}

void AMumulMumulGameMode::SpawnTent(const FTransform& SpawnTransform, int32 UserIndex)
{
	for (const TPair<TObjectPtr<ATentActor>, int32>& PoolElem : TentPool)
	{
		// If Owner has a tent
		if (PoolElem.Value == UserIndex && PoolElem.Key->bIsActive)
		{
			PoolElem.Key->ChangeTransform(SpawnTransform);
			PoolElem.Key->Mulicast_OnScaleAnimation();
			return;
		}
	}
	for (TPair<TObjectPtr<ATentActor>, int32>& PoolElem : TentPool)
	{
		if (!PoolElem.Key->bIsActive)
		{
			PoolElem.Key->SetOwnerUserIndex(UserIndex);
			PoolElem.Key->Activate(SpawnTransform);
			PoolElem.Key->Mulicast_OnScaleAnimation();
			PoolElem.Value = UserIndex;
			return;
		}
	}
	ATentActor* Tent = GetWorld()->SpawnActor<ATentActor>(TentClass);
	if (Tent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Tent Spawn FAILED to ADD"));
		return;
	}
	Tent->SetOwnerUserIndex(UserIndex);
	Tent->Activate(SpawnTransform);
	TentPool.Add(Tent, UserIndex);
}