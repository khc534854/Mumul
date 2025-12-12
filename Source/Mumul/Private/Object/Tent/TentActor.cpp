// Fill out your copyright notice in the Description page of Project Settings.


#include "Object/Tent/TentActor.h"

#include "Base/MumulGameState.h"
#include "Data/FHousingItemData.h"
#include "DynamicMesh/MeshTransforms.h"
#include "Object/CampFireActor.h"
#include "Library/MathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Object/HousingItemActor.h"
#include "Save/MapDataSaveGame.h"

static const FString HousingItemTablePath = TEXT("/Game/Khc/Blueprint/Object/HousingItemList.HousingItemList");
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
	DOREPLIFETIME(ATentActor, HousingItems);
	DOREPLIFETIME(ATentActor, OwnerUserIndex);
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

void ATentActor::OnRep_HousingItems()
{
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
    
	for (AActor* Child : AttachedActors)
	{
		if (Child->IsA(AHousingItemActor::StaticClass()))
		{
			Child->Destroy();
		}
	}

	for (const FHousingSaveData& Data : HousingItems)
	{
		SpawnHousingItem(Data);
	}
}

void ATentActor::Server_PlaceHousingItem(FName ItemID, FTransform Transform)
{
	for (int32 i = HousingItems.Num() - 1; i >= 0; --i)
	{
		if (HousingItems[i].ItemID == ItemID)
		{
			HousingItems.RemoveAt(i);
			break; 
		}
	}

	// 1-B. 월드에 소환된 실제 액터 제거
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	for (AActor* Child : AttachedActors)
	{
		AHousingItemActor* HItem = Cast<AHousingItemActor>(Child);
		if (HItem && HItem->ItemID == ItemID)
		{
			HItem->Destroy();
		}
	}

	// 2. [데이터 추가] 새로운 위치 정보 저장
	FHousingSaveData NewData;
	NewData.ItemID = ItemID;
	NewData.RelativeTransform = Transform;
    
	// 배열에 추가하면 ReplicatedUsing=OnRep_HousingItems에 의해 클라이언트들도 알게 됨
	HousingItems.Add(NewData); 

	// 3. [즉시 스폰] 서버에서도 바로 보이게 함
	SpawnHousingItem(NewData);

	// 4. [영구 저장] 게임 인스턴스/세이브 파일에 기록
	if (AMumulGameState* GS = GetWorld()->GetGameState<AMumulGameState>())
	{
		// 텐트 위치 + 하우징 아이템 목록 통째로 저장
		GS->Multicast_SaveTentData(OwnerUserIndex, GetActorTransform()); 
	}
}

void ATentActor::SpawnHousingItem(const FHousingSaveData& Data)
{
	UDataTable* Table = LoadObject<UDataTable>(nullptr, *HousingItemTablePath);
	if (!Table) return;

	FHousingItemData* ItemInfo = Table->FindRow<FHousingItemData>(Data.ItemID, TEXT("SpawnHousingItem"));
    
	// 데이터가 유효하고 메쉬 로드에 성공하면
	if (ItemInfo && ItemInfo->ItemStaticMesh.LoadSynchronous())
	{
		// 상대 좌표를 월드 좌표로 변환하여 스폰
		FTransform WorldTransform = Data.RelativeTransform * GetActorTransform();

		AHousingItemActor* NewItem = GetWorld()->SpawnActor<AHousingItemActor>(
			AHousingItemActor::StaticClass(), // BP가 있다면 해당 Class 사용 권장
			WorldTransform
		);
        
		if (NewItem)
		{
			// 정보 초기화 및 텐트에 부착 (텐트 이동 시 같이 이동됨)
			NewItem->InitHousingItem(Data.ItemID, OwnerUserIndex, ItemInfo->ItemStaticMesh.Get());
			NewItem->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
		}
	}
}

void ATentActor::Mulicast_OnScaleAnimation_Implementation()
{
	TentSequence1st = 0.f;
	TentSequence2nd = 0.f;
}

void ATentActor::SetOwnerUserIndex(int32 NewUserIndex)
{
	OwnerUserIndex = NewUserIndex;
}