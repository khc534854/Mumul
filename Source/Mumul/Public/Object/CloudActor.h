#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CloudActor.generated.h"

class UNiagaraSystem;
class UMaterialInstanceDynamic;

/* =====================
	Cloud FSM State
===================== */
UENUM(BlueprintType)
enum class ECloudState : uint8
{
	Appear,
	Idle,
	Charge,
	Fire,
	Recover,
	Disappear
};

UCLASS()
class MUMUL_API ACloudActor : public AActor
{
	GENERATED_BODY()

public:
	ACloudActor();

protected:
	/* =====================
		Engine
	===================== */
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY()
	TObjectPtr<class ACuteAlienPlayer> OwnerPlayer;
public:
	void InitOwnerPlayer(class ACuteAlienPlayer* Player) { OwnerPlayer = Player; }

protected:
	/* =====================
		FSM Core
	===================== */
	void EnterState(ECloudState NewState);
	void UpdateState(float DeltaTime);

	float GetStateAlpha(float Duration) const;
	ECloudState GetNextState(ECloudState Current) const;

	ECloudState State = ECloudState::Appear;
	float StateStartTime = 0.f;

	/* =====================
		State Update Logic
	===================== */
	void UpdateAppear();
	void UpdateIdle();
	void UpdateCharge();
	void UpdateFire();
	void UpdateRecover();
	void UpdateDisappear();

	/* =====================
		State Durations
	===================== */
	UPROPERTY(EditAnywhere, Category="Cloud|State")
	float AppearDuration = 0.8f;

	UPROPERTY(EditAnywhere, Category="Cloud|State")
	float IdleDuration = 1.1f;

	UPROPERTY(EditAnywhere, Category="Cloud|State")
	float ChargeDuration = 1.8f;

	UPROPERTY(EditAnywhere, Category="Cloud|State")
	float FireDuration = 0.5f;

	UPROPERTY(EditAnywhere, Category="Cloud|State")
	float RecoverDuration = 0.3f;

	UPROPERTY(EditAnywhere, Category="Cloud|State")
	float DisappearDuration = 0.4f;

	/* =====================
		Transform
	===================== */
	FVector StartLocation;
	FVector TargetLocation;

	// Scale control
	FVector IdleBaseScale = FVector(1.4f);
	FVector FireStartScale;
	FVector RecoverStartScale;
	FVector DisappearStartScale;

	// Disappear movement
	// Disappear (World Space)
	UPROPERTY(EditAnywhere, Category="Cloud|Disappear")
	FVector DisappearWorldDirection = FVector(1.f, 0.f, 0.f);
	FVector DisappearStartLocation;
	FVector DisappearDirection;

	/* =====================
		Material
	===================== */
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> CloudMID;

	static const FName MaskParamName;
	static const FName FlashParamName;
	static const FName ChargeParamName;
	
	/* =====================
		State Flags
	===================== */
	bool bFiredThisState = false;
	bool bAfterFire = false;
};
