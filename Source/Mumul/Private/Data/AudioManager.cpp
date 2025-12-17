// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/AudioManager.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	PlayIslandBGM();
}

void UAudioManager::PlayIslandBGM()
{
	if (IslandBGMComp)
		return;

	IslandBGMComp = UGameplayStatics::SpawnSound2D(
		this,
		IslandBGM,
		1.0f,
		1.0f,
		0.0f,
		nullptr,
		false
	);
}

void UAudioManager::StartQuizBGM()
{
	// 1. 상황 진입 → 믹스 Push
	UGameplayStatics::PushSoundMixModifier(GetWorld(), QuizSoundMix);

	// 2. 활동 BGM 재생
	if (!QuizBGMComp)
	{
		QuizBGMComp = UGameplayStatics::SpawnSound2D(
			GetWorld(),
			QuizBGM,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			false
		);
	}
}

void UAudioManager::EndQuizBGM()
{
	// 1. 활동 BGM 종료
	if (QuizBGMComp)
	{
		QuizBGMComp->FadeOut(0.3f, 0.0f);
		QuizBGMComp = nullptr;
	}

	// 2. 상황 종료 → 믹스 Pop
	UGameplayStatics::PopSoundMixModifier(GetWorld(), QuizSoundMix);
}
