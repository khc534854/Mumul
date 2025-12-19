// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/OXQuizTriggerActor.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Object/OXQuizActor.h"
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
	
	GlassMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GlassMesh"));
	GlassMesh->SetupAttachment(RootComponent);

	// DifficultyBubble = CreateDefaultSubobject<UWidgetComponent>(TEXT("DifficultyBubble"));
	// DifficultyBubble->SetupAttachment(SceneComp);
	// DifficultyBubble->SetVisibility(false);
}

// Called when the game starts or when spawned
void AOXQuizTriggerActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (GlassMesh)
	{
		GlassMID = GlassMesh->CreateDynamicMaterialInstance(0);
	}
	switch (QuizDifficulty)
	{
	case EQuizDifficulty::Beginner:
		GlassMID->SetScalarParameterValue(TEXT("Saturation"), 0.4f);
		break;

	case EQuizDifficulty::Intermediate:
		GlassMID->SetScalarParameterValue(TEXT("Saturation"), 0.81f);
		break;

	case EQuizDifficulty::Advanced:
		GlassMID->SetScalarParameterValue(TEXT("Saturation"), 0.94f);
		break;
	}
	

	OXQuizActor = Cast<AOXQuizActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AOXQuizActor::StaticClass()));

	OriginalLocation = GetActorLocation();
	Time = 0.f;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (PC->IsLocalController())
		{
			DifficultyBubble = NewObject<UWidgetComponent>(this, TEXT("DifficultyBubble"));
			DifficultyBubble->RegisterComponent();
			DifficultyBubble->AttachToComponent(
				SceneComp,
				FAttachmentTransformRules::KeepRelativeTransform
			);

			DifficultyBubble->SetWidgetSpace(EWidgetSpace::World);
			DifficultyBubble->SetDrawSize(FVector2D(350.f, 400.f));
			DifficultyBubble->SetVisibility(false);
			
			if (UDifficultyBubbleUI* Widget = Cast<UDifficultyBubbleUI>(DifficultyBubble->GetUserWidgetObject()))
			{
				const FText DifficultyText = GetDifficultyText();
				Widget->SetDifficultyText(DifficultyText);
			}
		}
	}
	
	// Give difference to each
	PhaseOffset = FMath::FRandRange(0.f, PI * 0.3f);
	AmplitudeScale = FMath::FRandRange(0.97f, 1.03f);
}

void AOXQuizTriggerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Hover
	Time += DeltaTime;
	float OffsetZ =	FMath::Sin(Time * HoverSpeed + PhaseOffset) * HoverAmplitude * AmplitudeScale;

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

FText AOXQuizTriggerActor::GetDifficultyText()
{
	switch (QuizDifficulty)
	{
	case EQuizDifficulty::Beginner:
		return FText::FromString(TEXT("초급"));
		
	case EQuizDifficulty::Intermediate:
		return FText::FromString(TEXT("중급"));

	case EQuizDifficulty::Advanced:
		return FText::FromString(TEXT("고급"));

	default:
		return FText::FromString(TEXT("초급"));
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
