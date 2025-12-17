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
	
	UPROPERTY(EditAnywhere, Category = "Sound|BGM")
	TObjectPtr<class USoundBase> IslandBGM;

	UPROPERTY(EditAnywhere, Category = "Sound|BGM")
	TObjectPtr<class USoundBase> QuizBGM;

	UPROPERTY(EditAnywhere, Category = "Sound|Mix")
	TObjectPtr<class USoundMix> QuizSoundMix;
	
	UPROPERTY()
	TObjectPtr<class UAudioComponent> IslandBGMComp;
	UPROPERTY()
	TObjectPtr<class UAudioComponent> QuizBGMComp;

public:
	void PlayIslandBGM();
	
	void StartQuizBGM();
	void EndQuizBGM();
};
