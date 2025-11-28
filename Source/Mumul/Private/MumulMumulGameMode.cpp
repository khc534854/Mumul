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

	UE_LOG(LogTemp, Warning, TEXT("========== [TentPool Init Check] =========="));
    
	int32 Index = 0;
	for (const TPair<TObjectPtr<ATentActor>, FString>& Pair : TentPool)
	{
		// 키(액터)의 이름 가져오기 (유효성 체크 포함)
		FString ActorName = (Pair.Key != nullptr) ? Pair.Key->GetName() : TEXT("NULL");
        
		// 밸류(주인 이름) 가져오기 (비어있으면 "None"으로 표시)
		FString OwnerName = Pair.Value.IsEmpty() ? TEXT("None") : Pair.Value;

		UE_LOG(LogTemp, Display, TEXT("[%d] Key(Actor): %s || Value(Owner): %s"), Index, *ActorName, *OwnerName);
        
		Index++;
	}
    
	UE_LOG(LogTemp, Warning, TEXT("==========================================="));
}

void AMumulMumulGameMode::SpawnTent(const FTransform& SpawnTransform, FString Name)
{
	UE_LOG(LogTemp, Warning, TEXT("========== [TentPool Spawn Check] =========="));
    
	int32 Index = 0;
	for (const TPair<TObjectPtr<ATentActor>, FString>& Pair : TentPool)
	{
		// 키(액터)의 이름 가져오기 (유효성 체크 포함)
		FString ActorName = (Pair.Key != nullptr) ? Pair.Key->GetName() : TEXT("NULL");
        
		// 밸류(주인 이름) 가져오기 (비어있으면 "None"으로 표시)
		FString OwnerName = Pair.Value.IsEmpty() ? TEXT("None") : Pair.Value;

		UE_LOG(LogTemp, Display, TEXT("[%d] Key(Actor): %s || Value(Owner): %s"), Index, *ActorName, *OwnerName);
        
		Index++;
	}
    
	UE_LOG(LogTemp, Warning, TEXT("==========================================="));
	
	
	for (const TPair<TObjectPtr<ATentActor>, FString>& PoolElem : TentPool)
	{
		//if (PoolElem.Value == Name)
		if (PoolElem.Value == Name && PoolElem.Key->bIsActive)
		{
			PoolElem.Key->SetOwnerName(FName(Name));
			PoolElem.Key->ChangeTransform(SpawnTransform);
			PoolElem.Key->Mulicast_OnScaleAnimation();
			
			UE_LOG(LogTemp, Warning, TEXT("========== [TentPool Spawn Again Check] =========="));
			Index = 0;
			for (const TPair<TObjectPtr<ATentActor>, FString>& Pair : TentPool)
			{
				// 키(액터)의 이름 가져오기 (유효성 체크 포함)
				FString ActorName = (Pair.Key != nullptr) ? Pair.Key->GetName() : TEXT("NULL");
        
				// 밸류(주인 이름) 가져오기 (비어있으면 "None"으로 표시)
				FString OwnerName = Pair.Value.IsEmpty() ? TEXT("None") : Pair.Value;

				UE_LOG(LogTemp, Display, TEXT("[%d] Key(Actor): %s || Value(Owner): %s"), Index, *ActorName, *OwnerName);
        
				Index++;
			}
    
			UE_LOG(LogTemp, Warning, TEXT("==========================================="));
			return;
		}
	}
	for (TPair<TObjectPtr<ATentActor>, FString>& PoolElem : TentPool)
	{
		if (!PoolElem.Key->bIsActive)
		{
			UE_LOG(LogTemp, Warning, TEXT("========== [TentPool Set Owner Check] =========="));
    
			Index = 0;
			for (const TPair<TObjectPtr<ATentActor>, FString>& Pair : TentPool)
			{
				// 키(액터)의 이름 가져오기 (유효성 체크 포함)
				FString ActorName = (Pair.Key != nullptr) ? Pair.Key->GetName() : TEXT("NULL");
        
				// 밸류(주인 이름) 가져오기 (비어있으면 "None"으로 표시)
				FString OwnerName = Pair.Value.IsEmpty() ? TEXT("None") : Pair.Value;

				UE_LOG(LogTemp, Display, TEXT("[%d] Key(Actor): %s || Value(Owner): %s"), Index, *ActorName, *OwnerName);
        
				Index++;
			}
    
			UE_LOG(LogTemp, Warning, TEXT("==========================================="));
			
			PoolElem.Key->SetOwnerName(FName(Name));
			PoolElem.Key->Activate(SpawnTransform);
			PoolElem.Key->Mulicast_OnScaleAnimation();
			PoolElem.Value = Name;
			return;
		}
	}
	ATentActor* Tent = GetWorld()->SpawnActor<ATentActor>(TentClass);
	if (Tent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Tent Spawn FAILED to ADD"));
		return;
	}
	Tent->SetOwnerName(FName(Name));
	Tent->Activate(SpawnTransform);
	TentPool.Add(Tent, Name);
}