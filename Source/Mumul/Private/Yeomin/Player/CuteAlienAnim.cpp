// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Player/CuteAlienAnim.h"

#include "Yeomin/Player/CuteAlienPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"

void UCuteAlienAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ACuteAlienPlayer>(TryGetPawnOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCuteAlienAnim, Owner is nullptr"))
	}
}

void UCuteAlienAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Owner)
	{
		FVector Velocity = Owner->GetVelocity();
		Velocity.Z = 0.f;
		CharacterSpeed = FMath::FInterpTo(CharacterSpeed, Velocity.Size(), DeltaSeconds, Smoothness);
	}
}

void UCuteAlienAnim::AnimNotify_StartJump()
{
	JumpPlayRate = 2.f;
}

void UCuteAlienAnim::AnimNotify_OnJump()
{
	if (Owner)
	{
		Owner->Jump();
	}
	JumpPlayRate = 1.f;
}
