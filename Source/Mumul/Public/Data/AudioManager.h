// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioManager.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UAudioManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	UPROPERTY()
	TObjectPtr<class USoundBase> IslandBGM;
	UPROPERTY()
	TObjectPtr<class USoundBase> QuizBGM;
	UPROPERTY()
	TObjectPtr<class USoundBase> FeedbackBGM;

	UPROPERTY()
	TObjectPtr<USoundBase> BeepSFX;
	UPROPERTY()
	TObjectPtr<USoundBase> EndBeepSFX;
	UPROPERTY()
	TObjectPtr<USoundBase> QuizEndSFX;
	UPROPERTY()
	TObjectPtr<class USoundMix> QuizSoundMix;
	
	UPROPERTY()
	TObjectPtr<USoundBase> CampfireSFX;
	
	UPROPERTY()
	TObjectPtr<class UAudioComponent> IslandBGMComp;
	UPROPERTY()
	TObjectPtr<class UAudioComponent> QuizBGMComp;
	UPROPERTY()
	TObjectPtr<class UAudioComponent> CampFireComp;
	UPROPERTY()
	TObjectPtr<class UAudioComponent> FeedbackComp;
	
	UPROPERTY()
	TObjectPtr<USoundBase> ClickSFX;
	UPROPERTY()
	TObjectPtr<USoundBase> PopUpSFX;
	UPROPERTY()
	TObjectPtr<USoundBase> PopDownSFX;
	UPROPERTY()
	TObjectPtr<USoundBase> NoticeSFX;

public:
	void PlayIslandBGM();
	
	void StartQuizBGM();
	void PlayBeepSound();
	void PlayEndBeepSound();
	void EndQuizBGM();
	
	void StartCampFireSound();
	void EndCampFireSound();
	
	void StartFeedbackMute();
	void EndFeedbackMute();
	
	void PlayClickSound();
	void PlayPopUpSound();
	void PlayPopDownSound();
	void PlayNoticeSound();
};
