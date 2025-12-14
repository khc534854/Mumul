// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OXQuizTriggerActor.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Object/OXQuizActor.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "UI/OXQuiz/DifficultyBubbleUI.h"


// Sets default values
AOXQuizTriggerActor::AOXQuizTriggerActor()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	TriggerSphere = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	TriggerSphere->SetupAttachment(RootComponent);
	TriggerSphere->OnComponentBeginOverlap.AddDynamic(this, &AOXQuizTriggerActor::OnBeginOverlapPlayer);
	TriggerSphere->OnComponentEndOverlap.AddDynamic(this, &AOXQuizTriggerActor::OnEndOverlapPlayer);

	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComp"));
	SceneComp->SetupAttachment(TriggerSphere);
	
	DifficultyBubble = CreateDefaultSubobject<UWidgetComponent>(TEXT("DifficultyBubble"));
	DifficultyBubble->SetupAttachment(SceneComp);
	DifficultyBubble->SetVisibility(false);
}

// Called when the game starts or when spawned
void AOXQuizTriggerActor::BeginPlay()
{
	Super::BeginPlay();

	OXQuizActor = Cast<AOXQuizActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AOXQuizActor::StaticClass()));

	OriginalLocation = GetActorLocation();
	Time = 0.f;

	if (UDifficultyBubbleUI* Widget = Cast<UDifficultyBubbleUI>(DifficultyBubble->GetUserWidgetObject()))
	{
		const FText DifficultyText = StaticEnum<EQuizDifficulty>()->GetDisplayNameTextByValue(
			static_cast<int32>(QuizDifficulty));
		Widget->SetDifficultyText(DifficultyText);
	}
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

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ACharacter* Character = PC->GetCharacter())
		{
			FVector PlayerLoc = Character->GetActorLocation();
			float DistSquare = FVector::DistSquared(PlayerLoc, this->GetActorLocation());

			if (DistSquare < FMath::Square(UIRotDistance))
			{
				if (DifficultyBubble)
				{
					FRotator CamRot = PC->PlayerCameraManager->GetCameraRotation();
					CamRot.Yaw += 180.f;
					CamRot.Pitch = 0.f;
					DifficultyBubble->SetWorldRotation(CamRot);
				}

				if (DistSquare < FMath::Square(LookAtDistance))
				{
					FVector Dir = PlayerLoc - GetActorLocation();
					FRotator TargetRot = Dir.Rotation();
					TargetRot.Pitch = 0.f;
					TargetRot.Roll = 0.f;

					FRotator NewLookRot = FMath::RInterpTo(
						GetActorRotation(),
						TargetRot,
						DeltaTime,
						1.3f // 회전 속도
					);
					SetActorRotation(NewLookRot);
				}
			}
		}
	}
}

void AOXQuizTriggerActor::OnBeginOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                               const FHitResult& SweepResult)
{
	if (ACuteAlienPlayer* Player = Cast<ACuteAlienPlayer>(OtherActor))
	{
		if (Player->IsLocallyControlled())
		{
			DifficultyBubble->SetVisibility(true);
		}
	}
}

void AOXQuizTriggerActor::OnEndOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ACuteAlienPlayer* Player = Cast<ACuteAlienPlayer>(OtherActor))
	{
		if (Player->IsLocallyControlled())
		{
			DifficultyBubble->SetVisibility(false);
		}
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
