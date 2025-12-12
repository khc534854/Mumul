// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OXQuizPlayerFinderActor.h"

#include "Base/MumulMumulGameMode.h"
#include "Components/SphereComponent.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"


// Sets default values
AOXQuizPlayerFinderActor::AOXQuizPlayerFinderActor()
{
	PrimaryActorTick.bCanEverTick = false;

	PlayerFinderSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PlayerFinderSphere"));
	PlayerFinderSphere->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AOXQuizPlayerFinderActor::BeginPlay()
{
	Super::BeginPlay();

	GM = Cast<AMumulMumulGameMode>(GetWorld()->GetAuthGameMode());
}


void AOXQuizPlayerFinderActor::CheckParticipatingPlayers()
{
	if (GM)
	{
		GM->ParticipatingPlayers.Empty();
		TArray<AActor*> OverlappingActors;
		PlayerFinderSphere->GetOverlappingActors(OverlappingActors, ACuteAlienPlayer::StaticClass());

		for (AActor* Actor : OverlappingActors)
		{
			ACuteAlienController* PC = Cast<ACuteAlienController>(Actor->GetInstigatorController());
			if (PC)
			{
				GM->ParticipatingPlayers.Add(PC);
			}
		}
	}
}
