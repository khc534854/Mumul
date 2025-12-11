// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OXQuizActor.h"

#include "Base/MumulMumulGameMode.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"


// Sets default values
AOXQuizActor::AOXQuizActor()
{
	PrimaryActorTick.bCanEverTick = true;

	LineArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LineArrow"));
	LineArrow->SetupAttachment(RootComponent);

	ParticipateBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ParticipateBox"));
	ParticipateBox->SetupAttachment(LineArrow);
	ParticipateBox->SetBoxExtent(FVector(50.f, 50.f, 10.f));
	ParticipateBox->SetRelativeLocation(FVector(0.f, 0.f, 10.f));
}

// Called when the game starts or when spawned
void AOXQuizActor::BeginPlay()
{
	Super::BeginPlay();

	GM = Cast<AMumulMumulGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->RegisterQuizActor(this);
	}
}

// Called every frame
void AOXQuizActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AOXQuizActor::Server_StartOXQuiz_Implementation(const FString& Question, const FString& Difficulty)
{
	// Set Participating Player
	CheckParticipatingPlayers();

	// TODO: 영역 가두기

	if (UHttpNetworkSubsystem* HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>())
	{
		HttpSystem->StartLearningQuizRequest(Question, Difficulty);
	}
}

void AOXQuizActor::CheckParticipatingPlayers()
{
	GM->ParticipatingPlayers.Empty();
	TArray<AActor*> OverlappingActors;
	ParticipateBox->GetOverlappingActors(OverlappingActors, ACuteAlienPlayer::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		ACuteAlienController* PC = Cast<ACuteAlienController>(Actor->GetInstigatorController());
		if (PC)
		{
			GM->ParticipatingPlayers.Add(PC);
		}
	}
}

void AOXQuizActor::JudgePlayerAnswers()
{
	if (GM)
	{
		FVector LineForward = LineArrow->GetForwardVector();
		FVector LineLocation = LineArrow->GetComponentLocation();

		for (TPair<TObjectPtr<ACuteAlienController>, TArray<bool>>& Elem : GM->ParticipatingPlayers)
		{
			FVector PlayerLocation = Elem.Key->GetCharacter()->GetActorLocation();

			// 내적 기반으로 선의 앞/뒤 판정
			FVector Dir = PlayerLocation - LineLocation;
			float Dot = FVector::DotProduct(LineForward, Dir);

			bool bIsO = Dot >= 0.f; // 예: 앞쪽 = O, 뒤쪽 = X

			// GameMode에 기록
			Elem.Value.Add(bIsO);
		}
	}
}
