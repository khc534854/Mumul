// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OXQuizActor.h"

#include "Base/MumulMumulGameMode.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Object/OXQuizPlayerFinderActor.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"


// Sets default values
AOXQuizActor::AOXQuizActor()
{
	PrimaryActorTick.bCanEverTick = false;

	LineArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("LineArrow"));
	LineArrow->SetupAttachment(RootComponent);
	
	FrontBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontBox"));
	FrontBox->SetupAttachment(LineArrow);
	FrontBox->SetBoxExtent(FVector(5.f, 50.f, 50.f));
	FrontBox->SetRelativeLocation(FVector(55.f, 0.f, 50.f));
	
	BackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BackBox"));
	BackBox->SetupAttachment(LineArrow);
	BackBox->SetBoxExtent(FVector(5.f, 50.f, 50.f));
	BackBox->SetRelativeLocation(FVector(-55.f, 0.f, 50.f));
	
	LeftBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBox"));
	LeftBox->SetupAttachment(LineArrow);
	LeftBox->SetBoxExtent(FVector(50.f, 5.f, 50.f));
	LeftBox->SetRelativeLocation(FVector(0.f, -55.f, 50.f));
	
	RightBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBox"));
	RightBox->SetupAttachment(LineArrow);
	RightBox->SetBoxExtent(FVector(50.f, 5.f, 50.f));
	RightBox->SetRelativeLocation(FVector(0.f, 55.f, 50.f));
}

// Called when the game starts or when spawned
void AOXQuizActor::BeginPlay()
{
	Super::BeginPlay();
	
	FrontBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GM = Cast<AMumulMumulGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->RegisterQuizActor(this);
	}

	OXQuizPlayerFinder = Cast<AOXQuizPlayerFinderActor>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AOXQuizPlayerFinderActor::StaticClass()));
}

void AOXQuizActor::StartOXQuiz(const int32 UserID, const FString& Difficulty)
{
	// Set Participating Player
	OXQuizPlayerFinder->CheckParticipatingPlayers();
	
	FrontBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BackBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LeftBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RightBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	if (GM)
	{
		for (TPair<TObjectPtr<ACuteAlienController>, TArray<bool>>& Elem : GM->ParticipatingPlayers)
		{
			Elem.Key->GetCharacter()->SetActorLocation(this->GetActorLocation() + FVector(0, 0, 100.f));
		}
	}

	if (UHttpNetworkSubsystem* HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>())
	{
		HttpSystem->StartLearningQuizRequest(UserID, Difficulty);
	}
}


void AOXQuizActor::JudgePlayerAnswers()
{
	if (GM)
	{
		FVector RightVector = LineArrow->GetRightVector();
		FVector LineLocation = LineArrow->GetComponentLocation();

		for (TPair<TObjectPtr<ACuteAlienController>, TArray<bool>>& Elem : GM->ParticipatingPlayers)
		{
			FVector PlayerLocation = Elem.Key->GetCharacter()->GetActorLocation();

			// 내적 기반으로 선의 오른쪽/왼쪽 판정
			FVector Dir = PlayerLocation - LineLocation;
			float Dot = FVector::DotProduct(RightVector, Dir);

			bool bIsO = Dot >= 0.f; // 예: 오른쪽 = O, 왼쪽 = X
			UE_LOG(LogTemp, Warning, TEXT("Player Dot is %d"), bIsO);
			// GameMode에 기록
			Elem.Value.Add(bIsO);
		}
	}
}
