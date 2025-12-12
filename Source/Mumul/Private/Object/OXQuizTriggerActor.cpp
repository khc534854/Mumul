// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OXQuizTriggerActor.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Object/OXQuizActor.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"


// Sets default values
AOXQuizTriggerActor::AOXQuizTriggerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(RootComponent);
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AOXQuizTriggerActor::OnBeginOverlapPlayer);
	TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AOXQuizTriggerActor::OnEndOverlapPlayer);
	
	DifficultyBubble = CreateDefaultSubobject<UWidgetComponent>(TEXT("DifficultyBubble"));
	DifficultyBubble->SetupAttachment(TriggerSphere);
}

// Called when the game starts or when spawned
void AOXQuizTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
	OXQuizActor = Cast<AOXQuizActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AOXQuizActor::StaticClass()));

	OriginalLocation = GetActorLocation();
	Time = 0.f;
}

void AOXQuizTriggerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Hover
	Time += DeltaTime;
	float OffsetZ = FMath::Sin(Time * HoverSpeed) * HoverAmplitude;

	FVector NewLocation = OriginalLocation;
	NewLocation.Z += OffsetZ;
	SetActorLocation(NewLocation);
	
	if (DifficultyBubble)
	{
		if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
		{
			FRotator CamRot = PC->PlayerCameraManager->GetCameraRotation();
			CamRot.Yaw += 180.f;
			DifficultyBubble->SetWorldRotation(CamRot);
		}
	}
}

void AOXQuizTriggerActor::OnBeginOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
		return;
	
	if (ACuteAlienPlayer* Player = Cast<ACuteAlienPlayer>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player BeginOverlap"))
		ACuteAlienController* PC = Cast<ACuteAlienController>(Player->GetController());
		PC->bIsNearEnoughToTrigger = true;
		
		// 키 가이드 UI 표시
	}
}

void AOXQuizTriggerActor::OnEndOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
		return;
	
	if (ACuteAlienPlayer* Player = Cast<ACuteAlienPlayer>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Player EndOverlap"))
		ACuteAlienController* PC = Cast<ACuteAlienController>(Player->GetController());
		PC->bIsNearEnoughToTrigger = false;
		
		// 키 가이드 UI 숨기기
	}
}

void AOXQuizTriggerActor::OnTriggerQuiz(const int32 UserID)
{
	switch (QuizDifficulty)
	{
	case EQuizDifficulty::Beginner:
		UE_LOG(LogTemp, Warning, TEXT("초급 퀴즈 시작"));
		OXQuizActor->StartOXQuiz(UserID, FString(TEXT("초급")));
		break;

	case EQuizDifficulty::Intermediate:
		UE_LOG(LogTemp, Warning, TEXT("중급 퀴즈 시작"));
		OXQuizActor->StartOXQuiz(UserID, FString(TEXT("중급")));
		break;

	case EQuizDifficulty::Advanced:
		UE_LOG(LogTemp, Warning, TEXT("고급 퀴즈 시작"));
		OXQuizActor->StartOXQuiz(UserID, FString(TEXT("고급")));
		break;
	}
}

