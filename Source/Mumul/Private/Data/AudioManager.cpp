// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/AudioManager.h"

#include "Components/AudioComponent.h"
#include "Data/ObjectAndClassFinder.h"
#include "Kismet/GameplayStatics.h"

void UAudioManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UObjectAndClassFinder* Finder = UObjectAndClassFinder::Get();
	IslandBGM = Finder->GetSound(TEXT("IslandBGM"));
	QuizBGM = Finder->GetSound(TEXT("QuizBGM"));
	FeedbackBGM = Finder->GetSound(TEXT("FeedbackBGM"));
	QuizSoundMix = Finder->GetSoundMix(TEXT("QuizSoundMix"));

	BeepSFX = Finder->GetSound(TEXT("Beep"));
	EndBeepSFX = Finder->GetSound(TEXT("EndBeep"));
	QuizEndSFX = Finder->GetSound(TEXT("QuizEnd"));
	
	CampfireSFX = Finder->GetSound(TEXT("Campfire"));
	
	ClickSFX = Finder->GetSound(TEXT("Click"));
	PopUpSFX = Finder->GetSound(TEXT("PopUp"));
	PopDownSFX = Finder->GetSound(TEXT("PopDown"));
	NoticeSFX = Finder->GetSound(TEXT("Notice"));
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
	if (IslandBGMComp)
	{
		IslandBGMComp->FadeOut(0.5f, 0.0f);
		IslandBGMComp = nullptr;
	}
	
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

void UAudioManager::PlayBeepSound()
{
	if (!BeepSFX) return;

	// 믹스 유지 상태에서 그냥 재생
	UGameplayStatics::PlaySound2D(
		GetWorld(),
		BeepSFX,
		1.0f
	);
}

void UAudioManager::PlayEndBeepSound()
{
	if (!EndBeepSFX) return;

	// 믹스 유지 상태에서 그냥 재생
	UGameplayStatics::PlaySound2D(
		GetWorld(),
		EndBeepSFX,
		1.0f
	);
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
	
	UGameplayStatics::PlaySound2D(
		GetWorld(),
		QuizEndSFX,
		1.0f
	);
	
	PlayIslandBGM();
}

void UAudioManager::StartCampFireSound()
{
	if (IslandBGMComp)
	{
		IslandBGMComp->FadeOut(0.6f, 0.0f);
		IslandBGMComp = nullptr;
	}
	
	if (!CampFireComp)
	{
		CampFireComp = UGameplayStatics::SpawnSound2D(
			GetWorld(),
			CampfireSFX,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			false
		);
	}
}

void UAudioManager::EndCampFireSound()
{
	if (CampFireComp)
	{
		CampFireComp->FadeOut(0.3f, 0.0f);
		CampFireComp = nullptr;
	}
	
	PlayIslandBGM();
}

void UAudioManager::StartFeedbackMute()
{
	if (IslandBGMComp)
	{
		IslandBGMComp->FadeOut(1.1f, 0.0f);
		IslandBGMComp = nullptr;
	}
	
	if (!FeedbackComp)
	{
		FeedbackComp = UGameplayStatics::SpawnSound2D(
			GetWorld(),
			FeedbackBGM,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			false
		);
	}
}

void UAudioManager::EndFeedbackMute()
{
	if (FeedbackComp)
	{
		FeedbackComp->FadeOut(0.3f, 0.0f);
		FeedbackComp = nullptr;
	}
	
	PlayIslandBGM();
}

void UAudioManager::PlayClickSound()
{
	UGameplayStatics::PlaySound2D(GetWorld(), ClickSFX);
}

void UAudioManager::PlayPopUpSound()
{
	UGameplayStatics::PlaySound2D(GetWorld(), PopUpSFX);
}

void UAudioManager::PlayPopDownSound()
{
	UGameplayStatics::PlaySound2D(GetWorld(), PopDownSFX);
}

void UAudioManager::PlayNoticeSound()
{
	UGameplayStatics::PlaySound2D(GetWorld(), NoticeSFX);
}
