// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MumulGameSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Mumul Game Subsystem Settings"))
class MUMUL_API UMumulGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Config, Category = "Network")
	FString BaseURL = TEXT("http://127.0.0.1:8000");

	UPROPERTY(EditAnywhere, Config, Category = "Network")
	int32 VoiceChunkLength = 60;
};
