// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/PlayerHousingSystemComponent.h"

#include "Data/FHousingItemData.h"
#include "Object/PreviewHousingItemActor.h"

#include "Base/MumulMumulGameMode.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Object/Tent/PreviewTentActor.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Object/Tent/TentActor.h"
#include "Player/MumulPlayerState.h"
#include "UI/PlayerUI.h"
#include "save/MapDataSaveGame.h"

static const FString HousingItemDataTablePath = TEXT("/Game/Khc/Blueprint/Object/HousingItemList.HousingItemList");

// Sets default values for this component's properties
UPlayerHousingSystemComponent::UPlayerHousingSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	static ConstructorHelpers::FClassFinder<APreviewTentActor> PreviewTentFinder(
			TEXT("/Game/Yeomin/Actors/Tent/BP_PreviewTent.BP_PreviewTent_C"));
	if (PreviewTentFinder.Succeeded())
	{
		PreviewTentClass = PreviewTentFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<APreviewHousingItemActor> PreviewHousingItemFinder(
	TEXT("/Game/Khc/Blueprint/Object/BP_PreviewHousingItemActor.BP_PreviewHousingItemActor_C"));
	if (PreviewHousingItemFinder.Succeeded())
	{
		PreviewHousingItemClass = PreviewHousingItemFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<ATentActor> TentFinder(
		TEXT("/Game/Yeomin/Actors/Tent/BP_Tent.BP_Tent_C"));
	if (TentFinder.Succeeded())
	{
		TentClass = TentFinder.Class;
	}
	

}


// Called when the game starts
void UPlayerHousingSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	owner = Cast<ACuteAlienController>(GetOwner());
	
	if (owner)
		player = Cast<ACuteAlienPlayer>(owner->GetPawn()); 
}


// Called every frame
void UPlayerHousingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                             FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!owner || !player) return;
	
	if (PreviewTent)
    {
		FHitResult HitRes;
       	FCollisionQueryParams CollisionParams;
       	CollisionParams.AddIgnoredActor(owner);
       	if (player)
       	{
       	   CollisionParams.AddIgnoredActor(player);
       	   TArray<AActor*> AttachedActors;
       	   player->GetAttachedActors(AttachedActors);
       	   CollisionParams.AddIgnoredActors(AttachedActors);
       	}
	   	
       	FVector Start, End;
       	FRotator CamRot;
       	float Dist = 1500.f;
       	owner->GetPlayerViewPoint(Start, CamRot);
       	End = Start + CamRot.Vector() * Dist;
	   	
       	bool bIsHit = GetWorld()->LineTraceSingleByChannel(
       	   HitRes, Start, End, ECC_Visibility, CollisionParams
       	);
	   	
       	if (bIsHit)
       	{
       	    // [로테이션 수정]
       	    // 1. 플레이어를 향하는 3D 벡터 계산 (2D 아님)
       	    FVector DirectionToPlayer = player->GetActorLocation() - HitRes.ImpactPoint;
	   	
       	    // 2. 이 벡터를 바닥 경사면(Normal) 위로 투영(Project) -> "경사면을 따라 플레이어를 보는 방향"
       	    FVector ProjectedForward = FVector::VectorPlaneProject(DirectionToPlayer, HitRes.ImpactNormal);
       	    ProjectedForward.Normalize();
	   	
       	    // 3. Z(Up)는 바닥 수직, X(Forward)는 투영된 방향으로 회전 생성
       	    // 이렇게 하면 옆으로 비틀어지는(Roll) 현상이 사라집니다.
       	    FRotator TargetRot = UKismetMathLibrary::MakeRotFromZX(HitRes.ImpactNormal, ProjectedForward);
       		TargetRot.Pitch = 0.0f;
       		TargetRot.Roll = 0.0f;
       		
	   	
       	    FTransform HitPointTransform(TargetRot, HitRes.ImpactPoint, FVector::OneVector);
       	    PreviewTent->SetActorTransform(HitPointTransform);
	   	
       	    if (owner->WasInputKeyJustPressed(EKeys::LeftMouseButton))
       	    {
       	       owner->OnClick(HitRes.ImpactPoint, TargetRot);
       	    }
       	}
    }

    // ====================================================================================
    // [2] 하우징 아이템 프리뷰 (Preview Housing Item)
    // ====================================================================================
    if (PreviewHousingItem)
    {
        FHitResult HitRes;
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(owner);
        if (player)
        {
            CollisionParams.AddIgnoredActor(player);
            TArray<AActor*> AttachedActors;
            player->GetAttachedActors(AttachedActors);
            CollisionParams.AddIgnoredActors(AttachedActors);
        }
        CollisionParams.AddIgnoredActor(PreviewHousingItem);

        FVector Start, End;
        FRotator CamRot;
        owner->GetPlayerViewPoint(Start, CamRot);
        End = Start + CamRot.Vector() * 1500.f;

        bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitRes, Start, End, ECC_WorldStatic, CollisionParams);

        if (bIsHit)
        {
            float HalfHeight = 0.f;
            UBoxComponent* Box = PreviewHousingItem->FindComponentByClass<UBoxComponent>();
            if (Box)
            {
               HalfHeight = Box->GetScaledBoxExtent().Z;
            }

            FVector LiftOffset = HitRes.ImpactNormal * HalfHeight;
            FVector FinalLocation = HitRes.ImpactPoint + LiftOffset;

            // [로테이션 수정 - 위와 동일한 논리 적용]
            FVector DirectionToPlayer = player->GetActorLocation() - HitRes.ImpactPoint;
            
            // 바닥 평면에 투영하여 기울어진 바닥에서도 자연스럽게 플레이어를 보게 함
            FVector ProjectedForward = FVector::VectorPlaneProject(DirectionToPlayer, HitRes.ImpactNormal);
            ProjectedForward.Normalize();

            FRotator TargetRot = UKismetMathLibrary::MakeRotFromZX(HitRes.ImpactNormal, ProjectedForward);
        	TargetRot.Pitch = 0.0f;
        	TargetRot.Roll = 0.0f;

            FTransform HitPointTransform(
             TargetRot, 
             FinalLocation, 
             FVector::OneVector
            );
            PreviewHousingItem->SetActorTransform(HitPointTransform);
        }
       
        // 입력 처리 부분 (기존 유지)
        if (owner->WasInputKeyJustPressed(EKeys::LeftMouseButton))
        {
            if (PreviewHousingItem->bIsPlaceable && PreviewHousingItem->CurrentTargetTent)
            {
                FTransform WorldTransform = PreviewHousingItem->GetActorTransform();
                FTransform TentTransform = PreviewHousingItem->CurrentTargetTent->GetActorTransform();
                FTransform RelativeTransform = WorldTransform.GetRelativeTransform(TentTransform);

                Server_PlaceHousingItem(PreviewHousingItem->CurrentTargetTent, SelectedItemID, RelativeTransform);
                
                StopPreviewHousingItem();
                
                if (AMumulCharacter* MyChar = Cast<AMumulCharacter>(player))
                {
                    MyChar->SetFirstPersonView(false);
                }
                if (owner->PlayerUI)
                {
                    owner->PlayerUI->ResetHousingSelection();
                }
            }
        }
        else if (owner->WasInputKeyJustPressed(EKeys::RightMouseButton))
        {
            StopPreviewHousingItem();
            if (AMumulCharacter* MyChar = Cast<AMumulCharacter>(player))
            {
                MyChar->SetFirstPersonView(false);
            }
             if (owner->PlayerUI)
            {
                owner->PlayerUI->ResetHousingSelection();
            }
        }
    }
}

void UPlayerHousingSystemComponent::ShowPreviewTent()
{
	if (player)
	{
		player->SetFirstPersonView(true);
	}
	
	// Deactivate Mouse Cursor
	owner->SetIgnoreLookInput(false);
	owner->SetShowMouseCursor(false);
	owner->SetInputMode(FInputModeGameOnly());

	// Spawn Preview Tent
	PreviewTent = GetWorld()->SpawnActor<APreviewTentActor>(
		PreviewTentClass,
		player->GetActorLocation(),
		FRotator::ZeroRotator
	);
}


void UPlayerHousingSystemComponent::ShowPreviewHousingItem(FName idx)
{
	if (idx.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("ShowPreviewHousingItem: ItemID is None!"));
		return;
	}

	// [방어 코드] 클래스가 없으면 중단
	if (!PreviewHousingItemClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ShowPreviewHousingItem: PreviewHousingItemClass is NULL! Check Blueprint."));
		return;
	}
	
	if (player)
	{
		player->SetFirstPersonView(true);
	}
	
	if (PreviewHousingItem)
	{
		PreviewHousingItem->Destroy();
		PreviewHousingItem = nullptr;
	}
	
	// Deactivate Mouse Cursor
	owner->SetIgnoreLookInput(false);
	owner->SetShowMouseCursor(false);
	owner->SetInputMode(FInputModeGameOnly());
	
	// Spawn Preview Tent
	UDataTable* HousingTable = LoadObject<UDataTable>(nullptr, *HousingItemDataTablePath);
	if (HousingTable)
	{
		FHousingItemData* ItemData = HousingTable->FindRow<FHousingItemData>(idx, TEXT("Housing Preview"));
        
		// 3. 데이터가 유효하고 메쉬가 있을 때만 스폰
		if (ItemData && ItemData->ItemStaticMesh.LoadSynchronous())
		{
			// 4. 프리뷰 액터 스폰 (이제 안전함)
			PreviewHousingItem = GetWorld()->SpawnActor<APreviewHousingItemActor>(
			   PreviewHousingItemClass,
			   player->GetActorLocation(),
			   FRotator::ZeroRotator
			);

			if (PreviewHousingItem)
			{
				if (AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>())
				{
					PreviewHousingItem->SetOwnerInfo(PS->PS_UserIndex);
				}
				
				// 5. 메쉬 설정
				PreviewHousingItem->SetPreviewMesh(ItemData->ItemStaticMesh.Get());
				SelectedItemID = idx; 
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Housing Item Data Not Found or Mesh is invalid for ID: %s"), *idx.ToString());
		}
	}
}

void UPlayerHousingSystemComponent::StopPreviewHousingItem()
{
	if (PreviewHousingItem)
	{
		PreviewHousingItem->Destroy();
		PreviewHousingItem = nullptr;
		SelectedItemID = NAME_None;
        
		owner->SetShowMouseCursor(true);
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		owner->SetInputMode(InputMode);
		owner->PlayerUI->ResetHousingSelection();
		owner->OnToggleMouse();
	}
}

void UPlayerHousingSystemComponent::Server_PlaceHousingItem_Implementation(class ATentActor* TargetTent, FName ItemID,
	FTransform RelativeTransform)
{
	if (TargetTent)
	{
		// 텐트에게 아이템 추가 위임
		TargetTent->Server_PlaceHousingItem(ItemID, RelativeTransform);
	}
}

void UPlayerHousingSystemComponent::Server_SpawnTent_Implementation(const FTransform& TentTransform)
{
	AMumulMumulGameMode* GM = GetWorld()->GetAuthGameMode<AMumulMumulGameMode>();
	if (GM)
	{
		AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>();
		if (PS)
		{
			GM->SpawnTent(TentTransform, PS->PS_UserIndex, true);
		}
	}
}

