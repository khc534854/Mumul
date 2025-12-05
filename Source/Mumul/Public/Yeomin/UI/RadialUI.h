// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RadialUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API URadialUI : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// Radial Widgets -> Add in NativeConstruct
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> Slot_0;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> Slot_1;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> Slot_2;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> Slot_3;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> Slot_4;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> Slot_5;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> Slot_6;

	UPROPERTY()
	TArray<TObjectPtr<class UBorder>> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Radial")
	float RadiusOffset = 200.f;
	float SpacingAngle;
	int32 CurrentIdx = -1;

	void ArrangeSlots();

	UPROPERTY()
	TObjectPtr<class APlayerController> PC;

	FVector2D LastPos;
	FVector2D StableDir;
	FVector2D SmoothedDelta;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Dot;

	bool bOnHovered = false;
	
	void UpdateSelectedSlot(int32 NewIndex);

public:
	// 현재 선택된 슬롯 번호 반환 (없으면 -1)
	int32 GetCurrentSelectedIndex() const { return CurrentIdx; }
};
