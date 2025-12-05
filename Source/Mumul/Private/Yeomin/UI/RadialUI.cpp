// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/RadialUI.h"

#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"

void URadialUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	Slots.Empty();

	// Adding Widget to Slots
	Slots.Add(Slot_0);
	Slots.Add(Slot_1);
	Slots.Add(Slot_2);
	Slots.Add(Slot_3);
	Slots.Add(Slot_4);
	Slots.Add(Slot_5);
	Slots.Add(Slot_6);

	SpacingAngle = 360.f / Slots.Num();

	ArrangeSlots();

	PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("RadialUI Can't Find PlayerController"))
	}
}

void URadialUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!PC)
		return;
	
	FVector2D CurPos;
	PC->GetMousePosition(CurPos.X, CurPos.Y);

	FVector2D RawDelta = CurPos - LastPos;
	LastPos = CurPos;

	float DeltaSize = RawDelta.Size();
	FVector2D RawDir = RawDelta.GetSafeNormal();

	float Strength = FMath::Clamp(DeltaSize * 0.00757f, 0.f, 1.f);

	StableDir = FMath::Lerp(StableDir, RawDir, Strength);

	if (StableDir.Size() < 0.00001f)
		return;

	if (Dot)
	{
		FVector2D Dir = StableDir.GetSafeNormal();
		constexpr float Radius = 60.f;
		Dot->SetRenderTranslation(Dir * Radius);

		float Angle = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X));
		Dot->SetRenderTransformAngle(Angle);
	}

	FVector2D TempDir = StableDir;
	TempDir *= -1.f;
	float Angle = FMath::Atan2(TempDir.Y, TempDir.X);
	float AngleDeg = FMath::RadiansToDegrees(Angle);
	AngleDeg = FMath::Fmod(AngleDeg + 360.f - 90.f, 360.f);

	int32 BestIdx = FMath::RoundToInt(AngleDeg / SpacingAngle) % Slots.Num();
	if (BestIdx == CurrentIdx)
		return;

	UpdateSelectedSlot(BestIdx);
	CurrentIdx = BestIdx;
}

void URadialUI::ArrangeSlots()
{
	const int32 SlotNum = Slots.Num();

	if (SlotNum == 0)
		return;

	for (int32 Idx = 0; Idx < SlotNum; Idx++)
	{
		if (UBorder* Border = Slots[Idx])
		{
			const float AngleDeg = SpacingAngle * Idx - 90.f;
			const float AngleRad = FMath::DegreesToRadians(AngleDeg);

			const float X = FMath::Cos(AngleRad) * RadiusOffset;
			const float Y = FMath::Sin(AngleRad) * RadiusOffset;

			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Border->Slot))
			{
				CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
				CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				CanvasSlot->SetPosition(FVector2D(X, Y));
			}
		}
	}
}

void URadialUI::UpdateSelectedSlot(int32 NewIndex)
{
	// Debugging Highlight
	for (int32 Idx = 0; Idx < Slots.Num(); Idx++)
	{
		Slots[Idx]->SetBrushColor(
			Idx == NewIndex ? FLinearColor(1, 1, 0, 1) : FLinearColor(1, 1, 1, 1)
		);
	}
}
