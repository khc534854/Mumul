// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CuteAlienAnim.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UCuteAlienAnim : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY()
	TObjectPtr<class ACuteAlienPlayer> Owner;

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float CharacterSpeed = 0.f;
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float Smoothness = 12.f;
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsFalling = false;
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsJumping = false;
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	bool bIsJumpStarted = false;
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float JumpPlayRate = 2.f;

	UFUNCTION()
	void AnimNotify_StartJump();
	UFUNCTION()
	void AnimNotify_OnJump();
};
