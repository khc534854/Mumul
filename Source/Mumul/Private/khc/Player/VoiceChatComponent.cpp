#include "khc/Player/VoiceChatComponent.h"

#include "khc/Player/MumulPlayerState.h"
#include "HttpNetworkSubsystem.h"
#include "MumulGameInstance.h"
#include "MumulGameSettings.h"
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

	const UMumulGameSettings* Settings = GetDefault<UMumulGameSettings>();
	if (Settings)
	{
		ChunkLength = Settings->VoiceChunkLength;
	}
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
			}, ChunkLength, true); // true: 반복 실행
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

	// 1. 타이머 및 캡처 정지 (기존 코드)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChunkTimerHandle);
	}
	AudioCapture.StopStream();
	AudioCapture.CloseStream();
	bIsRecording = false;

	UE_LOG(LogTemp, Log, TEXT("Voice Recording Stopped. Sending Last Chunk..."));

	// 2. [수정] 마지막 전송 요청 및 델리게이트 연결
	bWaitingForLastChunk = true; // 대기 상태 진입

	// HTTP 시스템 가져오기
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (UMumulGameInstance* MumulGI = Cast<UMumulGameInstance>(GI))
	{
		if (UHttpNetworkSubsystem* HttpSystem = MumulGI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			// [수정] AddDynamic -> AddUObject 로 변경
			HttpSystem->OnSendVoiceCompleteDelegate_LowLevel.AddUObject(this, &UVoiceChatComponent::OnLastChunkUploadComplete);

			SendCurrentChunk(true);
		}
	}
	else
	{
		// 만약 시스템이 없다면 바로 종료 알림 (예외 처리)
		OnRecordingStopped.Broadcast();
	}
}

void UVoiceChatComponent::SendCurrentChunk(bool bIsLast)
{
	if (PCMBuffer.Num() <= 0)
	{
		if (bIsLast && bWaitingForLastChunk)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Voice] No Data Left. Skipping Upload & Finishing Immediately."));
             
			// 가짜 응답 처리 (바로 종료 알림)
			// HttpRequestPtr, HttpResponsePtr는 nullptr로 넘겨도 무방함
			OnLastChunkUploadComplete(nullptr, nullptr, true);
		}
		return; 
	}

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
			HttpSystem->SendAudioChunk(WavData, GetCurrentMeetingID(), FString::FromInt(UserID), CurrentChunkIndex++, bIsLast);
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

void UVoiceChatComponent::OnLastChunkUploadComplete(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	if (!bWaitingForLastChunk) return;

	UE_LOG(LogTemp, Warning, TEXT("[VoiceComponent] Last Chunk Upload Completed. Success: %d"), bWasSuccessful);

	bWaitingForLastChunk = false;

	// [수정] RemoveDynamic -> RemoveAll 로 변경 (깔끔하게 해제)
	UGameInstance* GI = GetWorld()->GetGameInstance();
	if (UMumulGameInstance* MumulGI = Cast<UMumulGameInstance>(GI))
	{
		if (UHttpNetworkSubsystem* HttpSystem = MumulGI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			HttpSystem->OnSendVoiceCompleteDelegate_LowLevel.RemoveAll(this);
		}
	}

	OnRecordingStopped.Broadcast();
}
