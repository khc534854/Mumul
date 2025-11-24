// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VoiceChatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUMUL_API UVoiceChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVoiceChatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// 외부(캐릭터의 Input)에서 호출할 함수들
	UFUNCTION(BlueprintCallable, Category="Voice")
	void StartSpeaking();

	UFUNCTION(BlueprintCallable, Category="Voice")
	void StopSpeaking();
};
