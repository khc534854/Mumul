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
	
	static float EaseInSine(float x);
	
	static float EaseInCubic(float x);
	
	static float EaseOutCubic(float x);
	
	static float EaseInQuint(float x);
	static float EaseOutQuint(float x);
	
	static float EaseOutQuart(float x);
	
	static float EaseOutExpo(float x);
	
	static float EaseInOutQuad(float x);
	
	static float EaseInOutQuart(float x);
	
	static float EaseInOutExpo(float x);
};
