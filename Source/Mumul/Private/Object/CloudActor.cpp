#include "Object/CloudActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/CuteAlienPlayer.h"

const FName ACloudActor::MaskParamName(TEXT("MaskAlpha"));
const FName ACloudActor::FlashParamName(TEXT("Flash"));
const FName ACloudActor::ChargeParamName(TEXT("Charge"));

ACloudActor::ACloudActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	SetActorScale3D(IdleBaseScale);
}

void ACloudActor::BeginPlay()
{
	Super::BeginPlay();

	if (UStaticMeshComponent* Mesh = GetComponentByClass<UStaticMeshComponent>())
	{
		CloudMID = Mesh->CreateDynamicMaterialInstance(0);
	}

	TargetLocation = GetActorLocation();
	StartLocation = TargetLocation + FVector(0.f, 0.f, 165.f);
	SetActorLocation(StartLocation);

	EnterState(ECloudState::Appear);
}

void ACloudActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateState(DeltaTime);
}


/* =====================
	FSM Core
===================== */

void ACloudActor::EnterState(ECloudState NewState)
{
	State = NewState;
	StateStartTime = GetWorld()->GetTimeSeconds();

	switch (State)
	{
	case ECloudState::Fire:
		bFiredThisState = false;
		FireStartScale = GetActorScale3D();
		bAfterFire = true;

		if (CloudMID)
		{
			CloudMID->SetScalarParameterValue(ChargeParamName, 1.f);
			CloudMID->SetScalarParameterValue(FlashParamName, 1.f);
		}
		break;

	case ECloudState::Recover:
		RecoverStartScale = GetActorScale3D();
		break;

	case ECloudState::Disappear:
		DisappearStartLocation = GetActorLocation();
		DisappearDirection = DisappearWorldDirection.GetSafeNormal();
		DisappearStartScale = GetActorScale3D(); // ★ 핵심
		break;

	default:
		break;
	}
}

void ACloudActor::UpdateState(float)
{
	switch (State)
	{
	case ECloudState::Appear:
		UpdateAppear();
		if (GetStateAlpha(AppearDuration) >= 1.f)
			EnterState(GetNextState(State));
		break;

	case ECloudState::Idle:
		UpdateIdle();
		if (GetStateAlpha(IdleDuration) >= 1.f)
			EnterState(GetNextState(State));
		break;

	case ECloudState::Charge:
		UpdateCharge();
		if (GetStateAlpha(ChargeDuration) >= 1.f)
			EnterState(GetNextState(State));
		break;

	case ECloudState::Fire:
		UpdateFire();
		if (GetStateAlpha(FireDuration) >= 1.f)
			EnterState(GetNextState(State));
		break;

	case ECloudState::Recover:
		UpdateRecover();
		if (GetStateAlpha(RecoverDuration) >= 1.f)
			EnterState(GetNextState(State));
		break;

	case ECloudState::Disappear:
		UpdateDisappear();
		if (GetStateAlpha(DisappearDuration) >= 1.f)
		{
			// 연출 끝 -> 제거
			SetActorHiddenInGame(true);
			SetActorTickEnabled(false);
			SetActorEnableCollision(false);

			if (HasAuthority())
			{
				Destroy();
			}
		}
		break;
	}
}

float ACloudActor::GetStateAlpha(float Duration) const
{
	return FMath::Clamp(
		(GetWorld()->GetTimeSeconds() - StateStartTime) / Duration,
		0.f, 1.f
	);
}

ECloudState ACloudActor::GetNextState(ECloudState Current) const
{
	switch (Current)
	{
	case ECloudState::Appear: return ECloudState::Idle;
	case ECloudState::Idle: return bAfterFire ? ECloudState::Disappear : ECloudState::Charge;
	case ECloudState::Charge: return ECloudState::Fire;
	case ECloudState::Fire: return ECloudState::Recover;
	case ECloudState::Recover: return ECloudState::Idle;
	default: return ECloudState::Idle;
	}
}

/* =====================
	State Logic
===================== */

void ACloudActor::UpdateAppear()
{
	const float A = GetStateAlpha(AppearDuration);
	SetActorLocation(FMath::Lerp(StartLocation, TargetLocation, A));

	if (CloudMID)
		CloudMID->SetScalarParameterValue(MaskParamName, A);
}

void ACloudActor::UpdateIdle()
{
	const float T = GetWorld()->GetTimeSeconds();

	FVector Scale = IdleBaseScale;
	Scale.X += FMath::Sin(T * 2.f) * 0.03f;
	Scale.Z += FMath::Sin(T * 1.6f + 0.4f) * 0.04f;
	Scale.Y -= (Scale.X - IdleBaseScale.X) * 0.3f;

	SetActorScale3D(Scale);
}

void ACloudActor::UpdateCharge()
{
	const float A = GetStateAlpha(ChargeDuration);
	const float Charge = FMath::InterpEaseInOut(0.f, 1.f, A, 3.f);

	FVector Scale = IdleBaseScale;
	Scale.Z -= Charge * 0.4f;
	Scale.X += Charge * 0.25f;
	Scale.Y = Scale.X;

	SetActorScale3D(Scale);

	if (CloudMID)
		CloudMID->SetScalarParameterValue(ChargeParamName, Charge);
}

void ACloudActor::UpdateFire()
{
	const float A = GetStateAlpha(FireDuration);

	if (!bFiredThisState)
	{
		bFiredThisState = true;

		if (OwnerPlayer)
		{
			const FVector FireDir = (OwnerPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			OwnerPlayer->Server_PlayElectrocutedMontage(this->GetActorLocation(), FireDir);
		}
	}

	const float Kick = FMath::Exp(-A * 6.f);

	FVector Scale = FireStartScale;
	Scale.Z += Kick * 0.35f;
	Scale.X -= Kick * 0.12f;
	Scale.Y = Scale.X;

	SetActorScale3D(Scale);

	if (CloudMID)
	{
		float Flash = (A < 0.12f) ? 1.f : FMath::Exp(-(A - 0.12f) * 8.f);
		CloudMID->SetScalarParameterValue(FlashParamName, Flash < 0.05f ? 0.f : Flash);
	}
}

void ACloudActor::UpdateRecover()
{
	const float A = GetStateAlpha(RecoverDuration);
	const float Ease = FMath::InterpEaseOut(0.f, 1.f, A, 2.f);

	SetActorScale3D(
		FMath::Lerp(RecoverStartScale, IdleBaseScale, Ease)
	);

	if (CloudMID)
		CloudMID->SetScalarParameterValue(FlashParamName, 0.f);
}

void ACloudActor::UpdateDisappear()
{
	const float Alpha = GetStateAlpha(DisappearDuration);
	const float Ease = FMath::InterpEaseIn(0.f, 1.f, Alpha, 3.5f);

	// 이동
	const float ForwardDistance = 1200.f;
	const float UpDistance = 180.f;

	const FVector TargetOffset =
		DisappearDirection * ForwardDistance * Ease +
		FVector(0.f, 0.f, UpDistance * Ease);

	SetActorLocation(DisappearStartLocation + TargetOffset);

	// ★ 스케일: IdleBaseScale 기준 유지
	const FVector TargetScale = DisappearStartScale * FMath::Lerp(1.f, 0.25f, Ease);
	SetActorScale3D(TargetScale);

	// 마스크 닫힘
	if (CloudMID)
	{
		CloudMID->SetScalarParameterValue(MaskParamName, 1.f - Ease);
	}
}
