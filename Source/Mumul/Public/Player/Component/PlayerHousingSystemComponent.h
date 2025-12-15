// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHousingSystemComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUMUL_API UPlayerHousingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerHousingSystemComponent();
 
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	UPROPERTY()
	TSubclassOf<class APreviewTentActor> PreviewTentClass;
	UPROPERTY()
	TObjectPtr<class APreviewTentActor> PreviewTent;
	UPROPERTY()
	TSubclassOf<class ATentActor> TentClass;
	UPROPERTY()
	TObjectPtr<class ATentActor> Tent;

	UPROPERTY()
	TSubclassOf<class APreviewHousingItemActor> PreviewHousingItemClass;
	UPROPERTY()
	TObjectPtr<class APreviewHousingItemActor> PreviewHousingItem;
	UPROPERTY()
	TSubclassOf<class AHousingItemActor> HousingItemClass;
	UPROPERTY()
	TObjectPtr<class AHousingItemActor> HousingItem;

	FName SelectedItemID = NAME_None;

public:
	void ShowPreviewTent();
	void ShowPreviewHousingItem(FName idx);
	void StopPreviewHousingItem();
	

	UFUNCTION(Server, Reliable)
	void Server_SpawnTent(const FTransform& TentTransform);
	
	UFUNCTION(Server, Reliable)
	void Server_PlaceHousingItem(class ATentActor* TargetTent, FName ItemID, FTransform RelativeTransform);
	
	void StartHousingDeleteMode();
	void StopHousingDeleteMode();

	// [신규] 삭제 실행 (좌클릭 시 호출)
	void TryDeleteHousingItem();

	// [신규] 서버 삭제 요청 RPC
	UFUNCTION(Server, Reliable)
	void Server_DestroyHousingItem(class ATentActor* ParentTent, FName ItemID);

protected:
	bool bIsDeleteMode = false;

	// 현재 바라보고 있는(삭제 예정인) 아이템
	UPROPERTY()
	TObjectPtr<class AHousingItemActor> CurrentTargetItem;
	
	UPROPERTY()
	TObjectPtr<class ACuteAlienController> owner;
	UPROPERTY()
	TObjectPtr<class ACuteAlienPlayer> player;
};
