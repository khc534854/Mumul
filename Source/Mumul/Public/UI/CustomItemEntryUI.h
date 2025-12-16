// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomItemEntryUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCustomItemChecked, FName, ItemID, bool, bIsChecked);

UENUM(BlueprintType)
enum class EItemEntryType : uint8
{
	Custom, // 커스텀 아이템
	Housing // 하우징 아이템
};

UCLASS()
class MUMUL_API UCustomItemEntryUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 아이템 정보 및 UI 초기화
	void InitItem(FName ItemID, UTexture2D* Thumbnail, FString ItemName, class ACuteAlienPlayer* Player, EItemEntryType Type = EItemEntryType::Custom);

	void SetPlacedState(bool bPlaced);

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UCheckBox> ItemCheckBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UImage> ThumbnailImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> ItemNameText;
	
	// 아이템 고유 ID 저장
	FName CosmeticItemID;
	
	// 서버로 장착 요청을 보낼 플레이어 (Pawn/Controller 대신 Character를 사용)
	UPROPERTY()
	TObjectPtr<class ACuteAlienPlayer> OwningPlayerCharacter;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnCheckBoxStateChanged(bool bIsChecked);

	void SetItemCheckState(bool bNewState);
	
public:
	// 외부에서 바인딩할 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnCustomItemChecked OnItemChecked;
	
	// 시각적 상태 업데이트 함수
protected:
	void UpdateVisualState();
	// [신규] 내부 상태 변수들
	EItemEntryType EntryType;
	FText OriginalName;
	bool bIsPlaced = false; // 하우징 아이템 전용 (배치 여부)

};