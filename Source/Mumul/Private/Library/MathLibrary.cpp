// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/MathLibrary.h"

float UMathLibrary::EaseOutElastic(float x)
{
	const float c4 = (2 * UE_PI) / 3.f;

	if (x <= 0.f) return 0.f;
	if (x >= 1.f) return 1.f;

	// -10 : 진동 폭, 
	return FMath::Pow(2.f, -10.f * x) * FMath::Sin((x * 10.f - 0.75f) * c4) + 1.f;
}

float UMathLibrary::EaseOutBounce(float x)
{
	const float n1 = 7.5625f;
	const float d1 = 2.75f;
	if (x < 1.f / d1)
	{
		return n1 * x * x;
	}
	else if (x < 2.f / d1)
	{
		x -= 1.5f / d1;
		return n1 * x * x + 0.75f;
	}
	else if (x < 2.5f / d1)
	{
		x -= 2.25f / d1;
		return n1 * x * x + 0.9375f;
	}
	else
	{
		x -= 2.625f / d1;
		return n1 * x * x + 0.984375f;
	}
}
