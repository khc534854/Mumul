#include "khc/Player/VoiceChatComponent.h"

#include "khc/Player/MumulPlayerState.h"
#include "HttpNetworkSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Library/MumulVoiceFunctionLibrary.h"

UVoiceChatComponent::UVoiceChatComponent()
{
	PrimaryComponentTick.bCanEverTick = false; 
}


void UVoiceChatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UVoiceChatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AudioCapture.IsStreamOpen())
	{
		AudioCapture.CloseStream();
	}
	Super::EndPlay(EndPlayReason);
}

void UVoiceChatComponent::ToggleSpeaking()
{
	if (bIsSpeaking)
	{
		StopSpeaking();
		bIsSpeaking = false;
	}
	else
	{
		StartSpeaking();
		bIsSpeaking = true;
	}
}

void UVoiceChatComponent::ToggleRecording()
{
	if (bIsRecording)
	{
		StopRecording();
	}
	else
	{
		StartRecording();
	}
}

void UVoiceChatComponent::StartSpeaking()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return; 

	if (!OwnerPawn->IsLocallyControlled()) return;

	if(bIsSpeaking)
	{
		UE_LOG(LogTemp, Log, TEXT("[VoiceComponent] Already Talking.. Keep Talking"));
	}

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		UE_LOG(LogTemp, Log, TEXT("[VoiceComponent] Start Talking"));
		PC->StartTalking();
		bIsSpeaking = true;
		OnSpeakingStateChanged.Broadcast(true);
	}
}

void UVoiceChatComponent::StopSpeaking()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	if (!OwnerPawn->IsLocallyControlled()) return;

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		UE_LOG(LogTemp, Log, TEXT("[VoiceComponent] Stop Talking"));
		PC->StopTalking();
		bIsSpeaking = false;
		OnSpeakingStateChanged.Broadcast(false);
	}
}

void UVoiceChatComponent::StartRecording()
{
	if (bIsRecording) return;

	if (!bIsSpeaking)
		StartSpeaking();

	// 1. 초기화
	PCMBuffer.Reset();
	CurrentChunkIndex = 0; // 인덱스 초기화

	// 2. 캡처 시작 (기존 코드 유지)
	Audio::FAudioCaptureDeviceParams Params;
	Params.DeviceIndex = 0;
	Params.NumInputChannels = 1;

	bool bOpened = AudioCapture.OpenAudioCaptureStream(Params, 
	   [this](const void* InAudio, int32 NumFrames, int32 InNumChannels, int32 InSampleRate, double StreamTime, bool bOverFlow)
	   {
		  OnAudioCapture((const float*)InAudio, NumFrames, InNumChannels, InSampleRate);
	   },
	   1024
	);

	if (bOpened)
	{
		AudioCapture.StartStream();
		bIsRecording = true;
		OnRecordingStateChanged.Broadcast(true);
		UE_LOG(LogTemp, Log, TEXT("Voice Recording Started!"));

		// 3. [추가] 1분(60초)마다 자동으로 자르기 타이머 시작
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(ChunkTimerHandle, [this]()
			{
				// 타이머가 울리면 현재까지 쌓인 데이터를 보냄 (마지막 아님)
				SendCurrentChunk(false);
			}, 60.0f, true); // true: 반복 실행
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to open Audio Capture Stream!"));
	}
}

void UVoiceChatComponent::StopRecording()
{
	if (!bIsRecording) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChunkTimerHandle);
	}

	// 2. 캡처 중지
	AudioCapture.StopStream();
	AudioCapture.CloseStream();
	bIsRecording = false;
	OnRecordingStateChanged.Broadcast(false);

	UE_LOG(LogTemp, Log, TEXT("Voice Recording Stopped."));

	// 3. [추가] 남아있는 마지막 버퍼 전송 (마지막임 = true)
	SendCurrentChunk(true);
	

}

void UVoiceChatComponent::SendCurrentChunk(bool bIsLast)
{
	if (PCMBuffer.Num() <= 0) return; // 데이터 없으면 스킵

	// 1. WAV 변환
	TArray<uint8> WavData = UMumulVoiceFunctionLibrary::ConvertPCMToWAV(PCMBuffer, RecordingSampleRate, RecordingNumChannels);

	// 2. 서버 전송
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (GI)
	{
		if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			// UserID 가져오기
			int32 UserID = -1;
			if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
			{
				if (AMumulPlayerState* PS = Cast<AMumulPlayerState>(OwnerPawn->GetPlayerState()))
				{
					UserID = PS->PS_UserIndex;
				}
			}

			// 전송 (ChunkIndex 사용 후 증가)
			// HttpSystem->SendAudioChunk 함수에 bIsLast 인자 추가 필요 (아래 참고)
			//HttpSystem->SendAudioChunk(WavData, CurrentMeetingID, UserID, CurrentChunkIndex++);
			HttpSystem->SendAudioChunk(WavData, "meeting_20251125_d5c9b047", FString::FromInt(UserID), CurrentChunkIndex++);
		}
	}

	// 3. [중요] 버퍼 비우기 (다음 1분을 위해)
	PCMBuffer.Reset();
    
	UE_LOG(LogTemp, Log, TEXT("[Voice] Chunk Sent! Index: %d, Size: %d, IsLast: %s"), 
		CurrentChunkIndex - 1, WavData.Num(), bIsLast ? TEXT("True") : TEXT("False"));
}

void UVoiceChatComponent::OnAudioCapture(const float* InAudio, int32 InNumFrames, int32 InNumChannels,
                                         int32 InSampleRate)
{
	if (!bIsRecording) return;

	// 정보 저장 (나중에 WAV 헤더 만들 때 씀)
	RecordingSampleRate = InSampleRate;
	RecordingNumChannels = InNumChannels;

	// float(-1.0 ~ 1.0) 데이터를 int16(-32768 ~ 32767)으로 변환
	int32 SampleCount = InNumFrames * InNumChannels;
    
	// 버퍼에 추가
	for (int32 i = 0; i < SampleCount; i++)
	{
		float Sample = FMath::Clamp(InAudio[i], -1.0f, 1.0f);
		int16 PCM16 = (int16)(Sample * 32767.0f);
        
		// int16을 uint8 배열 2개로 쪼개서 저장 (Little Endian)
		PCMBuffer.Add((uint8)(PCM16 & 0xFF));
		PCMBuffer.Add((uint8)((PCM16 >> 8) & 0xFF));
	}
}
