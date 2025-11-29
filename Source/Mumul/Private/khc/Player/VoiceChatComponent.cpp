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

	// // 1. 버퍼 초기화
	// PCMBuffer.Reset();
	//
	// // 2. 캡처 파라미터 설정
	// Audio::FAudioCaptureDeviceParams Params;
	// Params.DeviceIndex = 0; // 기본 마이크
	// Params.NumInputChannels = 1; // 모노 녹음 권장 (STT용이면 모노가 좋음)
	//
	// // 3. 스트림 열기 & 콜백 등록
	// bool bOpened = AudioCapture.OpenAudioCaptureStream(Params, 
	// 	[this](const void* InAudio, int32 NumFrames, int32 InNumChannels, int32 InSampleRate, double StreamTime, bool bOverFlow)
	// 	{
	// 		// 캡처된 데이터를 처리하는 함수 호출
	// 		OnAudioCapture((const float*)InAudio, NumFrames, InNumChannels, InSampleRate);
	// 	},
	// 	1024 // 버퍼 사이즈
	// );
	//
	// if (bOpened)
	// {
	// 	AudioCapture.StartStream();
	// 	bIsRecording = true;
	// 	UE_LOG(LogTemp, Log, TEXT("Voice Recording Started!"));
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("Failed to open Audio Capture Stream!"));
	// }
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

	UE_LOG(LogTemp, Log, TEXT("Voice Recording Stopped."));

	// 3. [추가] 남아있는 마지막 버퍼 전송 (마지막임 = true)
	SendCurrentChunk(true);
	
	// // 1. 캡처 중지
	// AudioCapture.StopStream();
	// AudioCapture.CloseStream();
	// bIsRecording = false;
	//
	// UE_LOG(LogTemp, Log, TEXT("Voice Recording Stopped. Buffer Size: %d bytes"), PCMBuffer.Num());
	//
	// if (PCMBuffer.Num() > 0)
	// {
	// 	// WAV 변환 (기존 유지)
	// 	TArray<uint8> WavData = UMumulVoiceFunctionLibrary::ConvertPCMToWAV(PCMBuffer, RecordingSampleRate, RecordingNumChannels);
 //        
	// 	// 로컬 저장 (테스트용, 유지)
	// 	FString SavedPath;
	// 	UMumulVoiceFunctionLibrary::SaveWavFile(WavData, TEXT("TestRecord"), SavedPath);
	// 	// FString FileName = FString::Printf(TEXT("Record_%s"), *FDateTime::Now().ToString());
	// 	// UMumulVoiceFunctionLibrary::SaveWavFile(WavData, FileName, SavedPath);
	//
	// 	// [수정] 전송 로직 변경
	// 	UGameInstance* GI = GetWorld()->GetGameInstance();
	// 	if (GI)
	// 	{
	// 		if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
	// 		{
	// 			FString UserID = TEXT("UnknownUser");
 //             
	// 			// MeetingID (방 이름) 설정 
	// 			// (나중에는 PlayerState의 VoiceChannelID 등을 문자열로 변환해서 넣으면 됩니다)
	// 			FString MeetingID = TEXT("Lobby"); 
	//
	// 			if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	// 			{
	// 				if (APlayerState* PS = OwnerPawn->GetPlayerState())
	// 				{
	// 					UserID = PS->GetPlayerName();
 //                   
	// 					// 예시: 채널 ID를 방 이름으로 사용하려면 아래 주석 해제
	// 					// AMumulPlayerState* MyPS = Cast<AMumulPlayerState>(PS);
	// 					// if(MyPS) MeetingID = FString::FromInt(MyPS->VoiceChannelID);
	// 				}
	// 			}
	//
	// 			// 2. Chunk Index 설정
	// 			// (현재는 단발성 전송이므로 1로 고정하거나, 멤버 변수로 카운팅 가능)
	// 			int32 ChunkIndex = 1;
	//
	// 			// 3. [핵심] 파이썬 규격에 맞춘 함수 호출
	// 			// (기존 SendMultipartVoice 대신 SendAudioChunk 사용)
	// 			HttpSystem->SendAudioChunk(WavData, MeetingID, UserID, ChunkIndex);
	// 		}
	// 	}
	// }
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
			FString UserID = TEXT("UnknownUser");
			if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
			{
				if (APlayerState* PS = OwnerPawn->GetPlayerState())
				{
					UserID = PS->GetPlayerName();
				}
			}

			// 전송 (ChunkIndex 사용 후 증가)
			// HttpSystem->SendAudioChunk 함수에 bIsLast 인자 추가 필요 (아래 참고)
			//HttpSystem->SendAudioChunk(WavData, CurrentMeetingID, UserID, CurrentChunkIndex++); 
			HttpSystem->SendAudioChunk(WavData, "meeting_20251125_d5c9b047", "1", CurrentChunkIndex++); 
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
