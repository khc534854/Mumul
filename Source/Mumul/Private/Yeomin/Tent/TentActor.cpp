// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Tent/TentActor.h"

#include "INodeAndChannelMappings.h"
#include "DSP/MidiNoteQuantizer.h"
#include "DynamicMesh/MeshTransforms.h"
#include "Library/MathLibrary.h"
#include "Net/UnrealNetwork.h"


// Sets default values
ATentActor::ATentActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	AActor::SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ATentActor::BeginPlay()
{
	Super::BeginPlay();
}

void ATentActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATentActor, bIsActive)
}

// Called every frame
void ATentActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
		// ▼ Sequence 1 : 하늘에서 떨어져 "찰싹!" 눌리는 구간
		if (TentSequence1st < 1.f)
		{
			// 눌리는 시간을 늘리기 위해 속도 낮춤
			TentSequence1st = FMath::Clamp(TentSequence1st + DeltaTime * 0.8f, 0.f, 1.f);

			float StartXY = 0.2f;
			float StartZ  = 4.0f;

			float EndXY = 2.4f;
			float EndZ  = 0.55f;

			// 천천히 눌리는 느낌을 위해 XY/Z 둘 다 expo 그대로 활용
			float EaseXY = UMathLibrary::EaseInOutExpo(TentSequence1st);
			float EaseZ  = UMathLibrary::EaseOutExpo(TentSequence1st);

			float XY = FMath::Lerp(StartXY, EndXY, EaseXY);
			float Z  = FMath::Lerp(StartZ,  EndZ,  EaseZ);

			SetActorScale3D(FVector(XY, XY, Z));
			return;
		}

		// ▼ Sequence 2 : 탄성 튕김
		if (TentSequence2nd < 1.f)
		{
			TentSequence2nd = FMath::Clamp(TentSequence2nd + DeltaTime * 1.4f, 0.f, 1.f);

			float StartXY = 2.4f;
			float StartZ  = 0.55f;

			float End = 1.0f;

			float Ease = UMathLibrary::EaseOutElastic(TentSequence2nd);

			float XY = FMath::Lerp(StartXY, End, Ease);
			float Z  = FMath::Lerp(StartZ,  End, Ease);

			SetActorScale3D(FVector(XY, XY, Z));
		}
	


	
	
	// if (!FMath::IsNearlyEqual(TentSequence1st, 1.f, 0.1f))
	// {
	// 	TentSequence1st += 0.01f;
	// 	
	// 	float StartXY = 0.01f;
	// 	float StartZ = 10.f;
	// 	
	// 	float EndXY = 2.f;
	// 	float EndZ = 0.1f;
	// 	float EaseAlphaXY = UMathLibrary::EaseInOutExpo(TentSequence1st);
	// 	float EaseAlphaZ = UMathLibrary::EaseOutExpo(TentSequence1st);
	// 	float ScaleXY = FMath::Lerp(StartXY, EndXY, EaseAlphaXY);
	// 	float ScaleZ = FMath::Lerp(StartZ, EndZ, EaseAlphaZ);
	// 	SetActorScale3D(FVector(ScaleXY, ScaleXY, ScaleZ));
	// 	return;
	// }
	// if (!FMath::IsNearlyEqual(TentSequence2nd, 1.f, 0.1f))
	// {
	// 	TentSequence2nd += 0.01f;
	// 	
	// 	float StartXY = 2.f;
	// 	float StartZ = 0.1f;
	// 	
	// 	float End = 1.f;
	// 	float EaseAlpha = UMathLibrary::EaseOutElastic(TentSequence2nd);
	// 	float ScaleXY = FMath::Lerp(StartXY, End, EaseAlpha);
	// 	float ScaleZ = FMath::Lerp(StartZ, End, EaseAlpha);
	// 	SetActorScale3D(FVector(ScaleXY, ScaleXY, ScaleZ));
	// }
}

void ATentActor::Activate(const FTransform& SpawnTransform)
{
	SetActorTransform(SpawnTransform);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	bIsActive = true;
}

void ATentActor::ChangeTransform(const FTransform& SpawnTransform)
{
	SetActorTransform(SpawnTransform);
}

void ATentActor::Deactivate()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
	bIsActive = false;
}

void ATentActor::Mulicast_OnScaleAnimation_Implementation()
{
	TentSequence1st = 0.f;
	TentSequence2nd = 0.f;
}
