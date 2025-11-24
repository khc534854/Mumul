// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AudioCaptureCore.h"
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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// 외부(캐릭터의 Input)에서 호출할 함수들
	UFUNCTION(BlueprintCallable, Category="Voice")
	void StartSpeaking();

	UFUNCTION(BlueprintCallable, Category="Voice")
	void StopSpeaking();

	// 녹음 (신규 기능)
	UFUNCTION(BlueprintCallable, Category="Voice|Recording")
	void StartRecording();

	UFUNCTION(BlueprintCallable, Category="Voice|Recording")
	void StopRecording();

	UFUNCTION(BlueprintCallable, Category="Voice|Recording")
	bool IsRecording() const { return bIsRecording; }

private:
	// 오디오 캡처 객체
	Audio::FAudioCapture AudioCapture;
    
	// 캡처된 PCM 데이터 버퍼
	TArray<uint8> PCMBuffer;
    
	// 녹음 상태 플래그
	bool bIsRecording = false;

	// 캡처 정보 저장 (나중에 WAV 변환 시 필요)
	int32 RecordingSampleRate = 48000;
	int32 RecordingNumChannels = 1;

	// 오디오 데이터가 들어올 때 호출되는 콜백
	void OnAudioCapture(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate);
};
