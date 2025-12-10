// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CustomItemEntryUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCustomItemChecked, FName, ItemID, bool, bIsChecked);

UCLASS()
class MUMUL_API UCustomItemEntryUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 아이템 정보 및 UI 초기화
	void InitItem(FName ItemID, UTexture2D* Thumbnail, FString ItemName, class ACuteAlienPlayer* Player);

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

public:
	// 외부에서 바인딩할 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnCustomItemChecked OnItemChecked;
};