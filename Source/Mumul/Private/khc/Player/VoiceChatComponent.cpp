#include "khc/Player/VoiceChatComponent.h"

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

void UVoiceChatComponent::StartSpeaking()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return;

	if (!OwnerPawn->IsLocallyControlled()) return;

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		UE_LOG(LogTemp, Log, TEXT("[VoiceComponent] Start Talking"));
		PC->StartTalking();
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
	}
}

void UVoiceChatComponent::StartRecording()
{
	if (bIsRecording) return;

	// 1. 버퍼 초기화
	PCMBuffer.Reset();

	// 2. 캡처 파라미터 설정
	Audio::FAudioCaptureDeviceParams Params;
	Params.DeviceIndex = 0; // 기본 마이크
	Params.NumInputChannels = 1; // 모노 녹음 권장 (STT용이면 모노가 좋음)

	// 3. 스트림 열기 & 콜백 등록
	bool bOpened = AudioCapture.OpenAudioCaptureStream(Params, 
		[this](const void* InAudio, int32 NumFrames, int32 InNumChannels, int32 InSampleRate, double StreamTime, bool bOverFlow)
		{
			// 캡처된 데이터를 처리하는 함수 호출
			OnAudioCapture((const float*)InAudio, NumFrames, InNumChannels, InSampleRate);
		},
		1024 // 버퍼 사이즈
	);

	if (bOpened)
	{
		AudioCapture.StartStream();
		bIsRecording = true;
		UE_LOG(LogTemp, Log, TEXT("Voice Recording Started!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to open Audio Capture Stream!"));
	}
}

void UVoiceChatComponent::StopRecording()
{
	if (!bIsRecording) return;

	// 1. 캡처 중지
	AudioCapture.StopStream();
	AudioCapture.CloseStream();
	bIsRecording = false;

	UE_LOG(LogTemp, Log, TEXT("Voice Recording Stopped. Buffer Size: %d bytes"), PCMBuffer.Num());

	// // 2. WAV 변환 및 저장 (라이브러리 사용)
	// if (PCMBuffer.Num() > 0)
	// {
	// 	TArray<uint8> WavData = UMumulVoiceFunctionLibrary::ConvertPCMToWAV(PCMBuffer, RecordingSampleRate, RecordingNumChannels);
 //        
	// 	FString SavedPath;
	// 	// 파일명: Record_시간.wav
	// 	FString FileName = FString::Printf(TEXT("Record_%s"), *FDateTime::Now().ToString());
 //        
	// 	if (UMumulVoiceFunctionLibrary::SaveWavFile(WavData, FileName, SavedPath))
	// 	{
	// 		UE_LOG(LogTemp, Log, TEXT("WAV Saved Successfully: %s"), *SavedPath);
	// 	}
	// }

	// if (PCMBuffer.Num() > 0)
	// {
	// 	TArray<uint8> WavData = UMumulVoiceFunctionLibrary::ConvertPCMToWAV(PCMBuffer, RecordingSampleRate, RecordingNumChannels);
 //        
	// 	// 1. 로컬 파일 저장 (확인용, 필요 없으면 삭제 가능)
	// 	FString SavedPath;
	// 	UMumulVoiceFunctionLibrary::SaveWavFile(WavData, TEXT("TestRecord"), SavedPath);
	//
	// 	// 2. [핵심] 서브시스템을 통해 서버로 전송
	// 	UGameInstance* GI = GetWorld()->GetGameInstance();
	// 	if (GI)
	// 	{
	// 		if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
	// 		{
	// 			HttpSystem->SendVoiceDataToPython(WavData);
	// 		}
	// 	}
	// }

	if (PCMBuffer.Num() > 0)
	{
		// WAV 변환 (기존 유지)
		TArray<uint8> WavData = UMumulVoiceFunctionLibrary::ConvertPCMToWAV(PCMBuffer, RecordingSampleRate, RecordingNumChannels);
        
		// 로컬 저장 (테스트용, 유지)
		FString SavedPath;
		UMumulVoiceFunctionLibrary::SaveWavFile(WavData, TEXT("TestRecord"), SavedPath);

		// [수정] 전송 로직 변경
		UGameInstance* GI = GetWorld()->GetGameInstance();
		if (GI)
		{
			if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
			{
				// 1. 메타데이터 생성 (JSON 문자열 직접 조립)
				FString PlayerName = TEXT("Unknown");
                
				// PlayerState에서 이름 가져오기 시도
				if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
				{
					if (APlayerState* PS = OwnerPawn->GetPlayerState())
					{
						PlayerName = PS->GetPlayerName();
					}
				}

				// JSON 문자열 만들기
				FString MetaJson = FString::Printf(TEXT("{\"player_name\": \"%s\", \"room_id\": \"Lobby\"}"), *PlayerName);

				// 2. 멀티파트 전송 함수 호출
				HttpSystem->SendMultipartVoice(WavData, MetaJson);
			}
		}
	}
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
