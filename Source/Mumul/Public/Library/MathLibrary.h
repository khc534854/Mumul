// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MathLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UMathLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static float EaseOutElastic(float x);

	static float EaseOutBounce(float x);
};
