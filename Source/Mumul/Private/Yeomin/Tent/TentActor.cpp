// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/Tent/TentActor.h"

#include "INodeAndChannelMappings.h"
#include "DSP/MidiNoteQuantizer.h"
#include "DynamicMesh/MeshTransforms.h"
#include "khc/Object/CampFireActor.h"
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

	UChildActorComponent* ChildComp = FindComponentByClass<UChildActorComponent>();
	if (ChildComp)
	{
		ChildCampFire = Cast<ACampFireActor>(ChildComp->GetChildActor());

		if (ChildCampFire)
		{
			ChildCampFire->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
}

void ATentActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATentActor, bIsActive)
}


void ATentActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1st Sequence: Slime Effect
	if (TentSequence1st < 1.f)
	{
		float SequenceTime = 1.f;
		TentSequence1st = FMath::Clamp(TentSequence1st + DeltaTime * SequenceTime, 0.f, 1.f);

		float StartXY = 0.f;
		float StartZ = 6.22f;

		float EndXY = 1.84f;
		float EndZ = 0.19f;
		
		float EaseXY = UMathLibrary::EaseInOutExpo(TentSequence1st);
		float EaseZ = UMathLibrary::EaseOutExpo(TentSequence1st);

		float XY = FMath::Lerp(StartXY, EndXY, EaseXY);
		float Z = FMath::Lerp(StartZ, EndZ, EaseZ);

		SetActorScale3D(FVector(XY, XY, Z));
		return;
	}

	// 2nd Sequence: Elastic Bounce
	if (TentSequence2nd < 1.f)
	{
		float SequenceTime = 1.4f;
		TentSequence2nd = FMath::Clamp(TentSequence2nd + DeltaTime * SequenceTime, 0.f, 1.f);

		float StartXY = 1.84f;
		float StartZ = 0.19f;

		float End = 1.f;

		float Ease = UMathLibrary::EaseOutElastic(TentSequence2nd);

		float XY = FMath::Lerp(StartXY, End, Ease);
		float Z = FMath::Lerp(StartZ, End, Ease);

		SetActorScale3D(FVector(XY, XY, Z));
	}
}

void ATentActor::Activate(const FTransform& SpawnTransform)
{
	SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);
	bIsActive = true;

	if (ChildCampFire)
	{
		ChildCampFire->SetActorRelativeLocation(FVector(140, 130, 0));
	}

	ForceNetUpdate();
}

void ATentActor::ChangeTransform(const FTransform& SpawnTransform)
{
	SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::TeleportPhysics);
	
	if (ChildCampFire)
	{
		ChildCampFire->SetActorRelativeLocation(FVector(140, 130, 0));
	}

	ForceNetUpdate();
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

void ATentActor::SetOwnerUserIndex(int32 NewUserIndex)
{
	OwnerUserIndex = NewUserIndex;

	// if (ChildCampFire)
	// {
	// 	ChildCampFire->CampfireChannelID = NewUserIndex;
	// }
}