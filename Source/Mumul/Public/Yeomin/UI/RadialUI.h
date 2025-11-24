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
};
