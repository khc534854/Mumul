// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/TendencyActor.h"

#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Player/CuteAlienController.h"


// Sets default values
ATendencyActor::ATendencyActor()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	InteractionUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionUI"));
	InteractionUI->SetupAttachment(RootComponent);
	InteractionUI->SetWidgetSpace(EWidgetSpace::Screen);
	InteractionUI->SetVisibility(false);

	InteractionUI->SetIsReplicated(false); 
    
	InteractionUI->SetVisibility(false);
}

// Called when the game starts or when spawned
void ATendencyActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ATendencyActor::OnOverlapBegin);
		GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ATendencyActor::OnOverlapEnd);
	}

	if (InteractionUI)
	{
		InteractionUI->SetVisibility(false); // 기본 숨김
	}
	
	
	if (IdleMontage)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->AnimationData.bSavedLooping = true;
		GetMesh()->AnimationData.bSavedPlaying = true;
		GetMesh()->AnimationData.AnimToPlay = IdleMontage;
	}
	UpdateBodyMaterial(TendencyIndex);
}

// Called every frame
void ATendencyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATendencyActor::UpdateBodyMaterial(int32 TendencyIdx)
{
	if (PlayerBodyMaterials.Num() == 0) return;

	int32 MatIndexStart = 0;

	if (TendencyIdx <= 1)
	{
		MatIndexStart = 0;
	}
	else if (TendencyIdx >= 2 && TendencyIdx <= 5)
	{
		MatIndexStart = (TendencyIdx - 1) * 3;
	}

	if (PlayerBodyMaterials.IsValidIndex(MatIndexStart + 2))
	{
		GetMesh()->SetMaterial(0, PlayerBodyMaterials[MatIndexStart]);
		GetMesh()->SetMaterial(2, PlayerBodyMaterials[MatIndexStart + 1]);
		GetMesh()->SetMaterial(3, PlayerBodyMaterials[MatIndexStart + 2]);
	}
}

// Called to bind functionality to input
void ATendencyActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ATendencyActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (Pawn->IsLocallyControlled())
		{
			if (InteractionUI) InteractionUI->SetVisibility(true);
		}
	}	
}

void ATendencyActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (APawn* Pawn = Cast<APawn>(OtherActor))
	{
		if (Pawn->IsLocallyControlled())
		{
			if (InteractionUI) InteractionUI->SetVisibility(false);
		}
	}
}

