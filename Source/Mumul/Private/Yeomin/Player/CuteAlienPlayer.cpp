// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Player/CuteAlienPlayer.h"

#include "Yeomin/Player/CuteAlienAnim.h"


// Sets default values
ACuteAlienPlayer::ACuteAlienPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UAnimMontage> DanceMontageFinder(
		TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Montage_Dance.Montage_Dance"));
	if (DanceMontageFinder.Succeeded())
	{
		DanceMontage = DanceMontageFinder.Object;
	}
}

// Called when the game starts or when spawned
void ACuteAlienPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACuteAlienPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACuteAlienPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACuteAlienPlayer::PlayAlienDance()
{
	if (PlayerAnim->Montage_IsPlaying(DanceMontage))
		return;
	
	PlayerAnim->Montage_Play(DanceMontage);
}

