// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/MathLibrary.h"

float UMathLibrary::EaseOutElastic(float x)
{
	const float c4 = (2 * UE_PI) / 3.f;

	if (x <= 0.f) return 0.f;
	if (x >= 1.f) return 1.f;

	// -10 : 진동 폭, 
	float Amplitude = -10.f;
	return FMath::Pow(2.f, Amplitude * x) * FMath::Sin((x * 10.f - 0.75f) * c4) + 1.f;
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

float UMathLibrary::EaseInSine(float x)
{
	return 1.0f - FMath::Cos((x * PI) * 0.5f);
}

float UMathLibrary::EaseInCubic(float x)
{
	return x * x * x;
}

float UMathLibrary::EaseOutCubic(float x)
{
	return 1.0f - FMath::Pow(1.0f - x, 3.0f);
}

float UMathLibrary::EaseInQuint(float x)
{
	return x * x * x * x * x;
}

float UMathLibrary::EaseOutQuint(float x)
{
	return 1.f - FMath::Pow(1.f - x, 5.f);
}

float UMathLibrary::EaseOutQuart(float x)
{
	return 1.0f - FMath::Pow(1.0f - x, 4.0f);
}

float UMathLibrary::EaseOutExpo(float x)
{
	return (x == 1.0f) ? 1.0f : 1.0f - FMath::Pow(2.0f, -10.0f * x);
}

float UMathLibrary::EaseInOutQuad(float x)
{
	return (x < 0.5f)
		       ? (2.0f * x * x)
		       : (1.0f - FMath::Pow(-2.0f * x + 2.0f, 2.0f) * 0.5f);
}

float UMathLibrary::EaseInOutQuart(float x)
{
	return (x < 0.5f)
		       ? (8.0f * x * x * x * x)
		       : (1.0f - FMath::Pow(-2.0f * x + 2.0f, 4) * 0.5f);
}

float UMathLibrary::EaseInOutExpo(float x)
{
	if (x <= 0.0f)
		return 0.0f;
	if (x >= 1.0f)
		return 1.0f;

	return (x < 0.5f)
		       ? FMath::Pow(2.0f, 20.0f * x - 10.0f) * 0.5f
		       : (2.0f - FMath::Pow(2.0f, -20.0f * x + 10.0f)) * 0.5f;
}
