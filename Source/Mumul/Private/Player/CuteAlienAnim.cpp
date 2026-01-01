// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CuteAlienAnim.h"

#include "Kismet/GameplayStatics.h"
#include "Player/CuteAlienPlayer.h"

void UCuteAlienAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 중복 호출 제거 및 안전한 캐스팅
	APawn* PawnOwner = TryGetPawnOwner();
	if (PawnOwner)
	{
		Owner = Cast<ACuteAlienPlayer>(PawnOwner);
	}
}

void UCuteAlienAnim::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// Owner가 nullptr이거나 유효하지 않으면 즉시 중단
	if (!IsValid(Owner))
	{
		// 런타임 중에 Owner가 사라졌을 수 있으므로 다시 시도
		APawn* PawnOwner = TryGetPawnOwner();
		if (PawnOwner)
		{
			Owner = Cast<ACuteAlienPlayer>(PawnOwner);
		}
		
		if (!IsValid(Owner)) return;
	}
	
	HeadYaw = Owner->GetLookYaw();
	HeadPitch = Owner->GetLookPitch();

	FVector Velocity = Owner->GetVelocity();
	Velocity.Z = 0.f;
	CharacterSpeed = FMath::FInterpTo(CharacterSpeed, Velocity.Size(), DeltaSeconds, Smoothness);
}

void UCuteAlienAnim::AnimNotify_StartJump()
{
	JumpPlayRate = 2.f;
}

void UCuteAlienAnim::AnimNotify_OnJump()
{
	if (Owner)
	{
		if (Owner->IsLocallyControlled())
		{
			UGameplayStatics::PlaySound2D(
				this,
				JumpSound
			);
		}

		Owner->Jump();
	}
	JumpPlayRate = 1.f;
}
