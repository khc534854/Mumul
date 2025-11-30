// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AudioCaptureCore.h"
#include "Components/ActorComponent.h"
#include "VoiceChatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceStateChanged, bool, bActive);


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
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void ToggleSpeaking();

	UFUNCTION(BlueprintCallable, Category = "Voice")
	void ToggleRecording();

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

	UFUNCTION(BlueprintCallable, Category = "Voice|Recording")
	bool IsSpeaking() const { return bIsSpeaking; }


	UFUNCTION(BlueprintCallable, Category = "Voice|Recording")
	FString GetCurrentMeetingID() const { return CurrentMeetingID; }
	
	UFUNCTION(BlueprintCallable, Category = "Voice|Recording")
	void SetCurrentMeetingID(FString NewMeetingId) { CurrentMeetingID = NewMeetingId; }

private:
	// 오디오 캡처 객체
	Audio::FAudioCapture AudioCapture;
    
	// 캡처된 PCM 데이터 버퍼
	TArray<uint8> PCMBuffer;
    
	// 녹음 상태 플래그
	bool bIsRecording = false;
	bool bIsSpeaking = false;

	// 캡처 정보 저장 (나중에 WAV 변환 시 필요)
	int32 RecordingSampleRate = 48000;
	int32 RecordingNumChannels = 1;

	FTimerHandle ChunkTimerHandle;

	// 현재 청크 인덱스
	int32 CurrentChunkIndex = 0;
	FString CurrentMeetingID = TEXT("Test");

	void SendCurrentChunk(bool bIsLast);
	
	// 오디오 데이터가 들어올 때 호출되는 콜백
	void OnAudioCapture(const float* InAudio, int32 InNumFrames, int32 InNumChannels, int32 InSampleRate);

public:
	// 외부(UI)에서 바인딩할 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Voice|Event")
	FOnVoiceStateChanged OnSpeakingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Voice|Event")
	FOnVoiceStateChanged OnRecordingStateChanged;
};
