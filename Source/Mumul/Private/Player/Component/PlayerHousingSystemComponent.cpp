// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/PlayerHousingSystemComponent.h"

#include "Base/MumulGameState.h"
#include "Data/FHousingItemData.h"
#include "Object/PreviewHousingItemActor.h"

#include "Base/MumulMumulGameMode.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Object/Tent/PreviewTentActor.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Object/HousingItemActor.h"
#include "Object/Tent/TentActor.h"
#include "Player/MumulPlayerState.h"
#include "UI/PlayerUI.h"
#include "save/MapDataSaveGame.h"

static const FString HousingDataTablePath = TEXT("/Game/Khc/Blueprint/Object/HousingItemList.HousingItemList");

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

            	if (owner->PlayerUI)
            	{
            		owner->PlayerUI->MarkHousingItemAsPlaced(SelectedItemID, true);
            		owner->PlayerUI->ResetHousingSelection(); // 체크 해제 -> "배치됨" 텍스트 표시
            	}
            	
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
	
    if (bIsDeleteMode)
    {
        FHitResult HitRes;
        FVector Start, End;
        FRotator CamRot;
        owner->GetPlayerViewPoint(Start, CamRot);
        End = Start + CamRot.Vector() * 1500.f;

        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(owner);
        CollisionParams.AddIgnoredActor(player);

        CollisionParams.bTraceComplex = true;

        // 1. 레이저 발사
        bool bIsHit = GetWorld()->LineTraceSingleByChannel(HitRes, Start, End, ECC_Visibility, CollisionParams);

        // 2. [수정] 타겟 후보 선정 (유효성 및 소유권 검사)
        AHousingItemActor* NewTarget = nullptr;

        if (bIsHit)
        {
            AHousingItemActor* HitItem = Cast<AHousingItemActor>(HitRes.GetActor());
            if (HitItem)
            {
                if (AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>())
                {
                    // 내 아이템일 경우에만 타겟 후보로 등록
                    if (HitItem->OwnerUserIndex == PS->PS_UserIndex)
                    {
                        NewTarget = HitItem;
                    }
                }
            }
        }

        // 3. [수정] 타겟 변경 시에만 하이라이트 상태 갱신 (로직 안정화)
        if (CurrentTargetItem != NewTarget)
        {
            // 기존 타겟 끄기
            if (CurrentTargetItem)
            {
                CurrentTargetItem->SetHighlightState(false);
            }

            // 새 타겟 교체
            CurrentTargetItem = NewTarget;

            // 새 타겟 켜기
            if (CurrentTargetItem)
            {
                CurrentTargetItem->SetHighlightState(true);
            }
        }

        // 4. 입력 처리
        if (owner->WasInputKeyJustPressed(EKeys::LeftMouseButton))
        {
            if (CurrentTargetItem)
            {
                TryDeleteHousingItem();
            }
        }
        else if (owner->WasInputKeyJustPressed(EKeys::RightMouseButton))
        {
            StopHousingDeleteMode();
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
	UDataTable* HousingTable = LoadObject<UDataTable>(nullptr, *HousingDataTablePath);
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
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		owner->SetInputMode(InputMode);
		owner->PlayerUI->ResetHousingSelection();
		owner->OnToggleMouse();
	}
}

void UPlayerHousingSystemComponent::StartHousingDeleteMode()
{
    // 기존 프리뷰가 있다면 종료
    if (PreviewTent) owner->OnCancelUI(); // (함수명이 있다면)

    bIsDeleteMode = true;
    CurrentTargetItem = nullptr;

    // 1인칭 시점 전환
    if (player)
    {
        player->SetFirstPersonView(true);
    }

    // 마우스 숨김 및 게임 입력 모드
    owner->SetIgnoreLookInput(false);
    owner->SetShowMouseCursor(false);
    owner->SetInputMode(FInputModeGameOnly());
}

void UPlayerHousingSystemComponent::StopHousingDeleteMode()
{
    bIsDeleteMode = false;

    // 타겟팅 해제 및 색상 복구
    if (CurrentTargetItem)
    {
        CurrentTargetItem->SetHighlightState(false);
        CurrentTargetItem = nullptr;
    }

    // 3인칭 복구 & 마우스 표시
    if (player)
    {
        player->SetFirstPersonView(false);
    }
    
    owner->SetShowMouseCursor(true);
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
    owner->SetInputMode(InputMode);
    
    // UI 체크박스 해제 요청 (필요하다면)
    if (owner->PlayerUI)
    {
        owner->PlayerUI->ResetHousingSelection();
    }
    owner->OnToggleMouse(); // 상황에 따라 필요
}

void UPlayerHousingSystemComponent::TryDeleteHousingItem()
{
    // 1. 타겟 확인
    if (!CurrentTargetItem) return;

    // 2. 부모 텐트 확인
    // (아이템이 붙어있는 부모 액터를 찾습니다)
    ATentActor* ParentTent = Cast<ATentActor>(CurrentTargetItem->GetAttachParentActor());

    // [중요] 만약 로컬에서 OnRep으로 생성된 아이템이라 부모가 텐트가 아닐 수도 있습니다.
    // 이 경우 Owner 등을 통해 어떻게든 텐트를 찾아야 하지만, 
    // 현재 구조상 GetAttachParentActor가 가장 확실합니다.
    if (!ParentTent)
    {
        // 부모를 못 찾았다면, 삭제 모드만 끄고 리턴 (안전 장치)
        UE_LOG(LogTemp, Warning, TEXT("[Client] Delete Failed: Parent Tent not found via Attachment."));
        StopHousingDeleteMode();
        return;
    }

    // 3. 로그 출력
    UE_LOG(LogTemp, Warning, TEXT("[Client] Requesting Delete Item: %s (ID: %s)"),
        *CurrentTargetItem->GetName(), *CurrentTargetItem->ItemID.ToString());

    // 4. 서버에 삭제 요청 (텐트와 아이템 ID만 전달 - 위치 비교 X)
    Server_DestroyHousingItem(ParentTent, CurrentTargetItem->ItemID);

	if (owner->PlayerUI)
	{
		owner->PlayerUI->MarkHousingItemAsPlaced(CurrentTargetItem->ItemID, false);
	}
	

    // 5. [핵심 수정] 클라이언트 측 '즉시 파괴'
    // 서버가 지우기 전에 클라이언트 눈앞에 있는 이 아이템(CurrentTargetItem)을 바로 없앱니다.
    // 이렇게 해야 OnRep으로 생긴 로컬 중복 아이템이 확실히 사라집니다.
    CurrentTargetItem->Destroy();

    StopHousingDeleteMode();
}

void UPlayerHousingSystemComponent::Server_DestroyHousingItem_Implementation(ATentActor* ParentTent, FName ItemID)
{
    // 1. 텐트 유효성 검사
    if (!ParentTent)
    {
        UE_LOG(LogTemp, Error, TEXT("[Server] Delete Failed: ParentTent is null"));
        return;
    }

    // 2. 소유권 확인 (텐트 주인이 요청자와 같은지)
    AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>();
    if (!PS || ParentTent->OwnerUserIndex != PS->PS_UserIndex)
    {
        UE_LOG(LogTemp, Error, TEXT("[Server] Delete Failed: Owner Mismatch!"));
        return;
    }

    // 3. 텐트 데이터(HousingItems)에서 해당 아이템 찾아서 제거
    bool bDataFound = false;
    for (int32 i = 0; i < ParentTent->HousingItems.Num(); ++i)
    {
        // 위치 비교 없이 ID만 같으면 삭제 (요청 사항 반영)
        if (ParentTent->HousingItems[i].ItemID == ItemID)
        {
            ParentTent->HousingItems.RemoveAt(i);
            bDataFound = true;

            // "같은 이름은 하나만 설치"한다고 했으므로 찾으면 바로 break
            break;
        }
    }

    // 4. 실제 스폰된 액터(Attached Actor) 찾아서 파괴
    // 서버 월드에 존재하는 '서버판 아이템'을 찾아서 지웁니다.
    bool bActorFound = false;
    TArray<AActor*> Children;
    ParentTent->GetAttachedActors(Children);

    for (AActor* Child : Children)
    {
        AHousingItemActor* HItem = Cast<AHousingItemActor>(Child);
        if (HItem)
        {
            // ID만 비교하여 일치하면 파괴
            if (HItem->ItemID == ItemID)
            {
                HItem->Destroy();
                bActorFound = true;
                // 중복 설치 방지 규칙에 따라 하나만 지우고 break
                break;
            }
        }
    }

    // 5. 결과 저장 및 로그
    if (bDataFound)
    {
        if (AMumulGameState* GS = GetWorld()->GetGameState<AMumulGameState>())
        {
            // 변경된 데이터 저장
            GS->Multicast_SaveTentData(ParentTent->OwnerUserIndex, ParentTent->OwnerName, ParentTent->GetActorTransform());
            UE_LOG(LogTemp, Log, TEXT("[Server] Delete Success. Data Removed. (Actor Destroyed: %s)"), bActorFound ? TEXT("Yes") : TEXT("No"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Server] Data item not found for ID: %s. (Actor Destroyed: %s)"), *ItemID.ToString(), bActorFound ? TEXT("Yes") : TEXT("No"));
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
			UE_LOG(LogTemp, Warning, TEXT("[Server_SpawnTent] UserIndex: %d, Name: %s"), PS->PS_UserIndex, *PS->PS_RealName);
			GM->SpawnTent(TentTransform, PS->PS_UserIndex, PS->PS_RealName, true);
		}
	}
}

