// Fill out your copyright notice in the Description page of Project Settings.


#include "khc/System/MumulVoiceFunctionLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

// PCM(Raw Audio) 데이터를 WAV 파일 포맷(헤더 포함)으로 변환하는 함수
TArray<uint8> UMumulVoiceFunctionLibrary::ConvertPCMToWAV(const TArray<uint8>& InPCMData, int32 SampleRate,
    int32 NumChannels)
{
    TArray<uint8> WavData; // [필수] 최종 WAV 데이터를 담을 배열 선언

    // --- WAV 헤더 계산 영역 (WAV 표준 규격) ---
    int32 BitsPerSample = 16; // [설정] 비트 깊이 (보통 16비트 사용. 변경 가능하지만 수신측과 맞춰야 함)
    int32 DataSize = InPCMData.Num(); // [필수] 순수 오디오 데이터의 크기
    int32 ChunkSize = 36 + DataSize; // [필수] 전체 파일 크기에서 앞의 8바이트를 뺀 크기 계산
    int32 ByteRate = SampleRate * NumChannels * (BitsPerSample / 8); // [필수] 1초당 전송되는 바이트 수 계산
    int32 BlockAlign = NumChannels * (BitsPerSample / 8); // [필수] 샘플 프레임의 크기 계산

    // --- RIFF Chunk 작성 (파일 형식 식별) ---
    WavData.Append((const uint8*)"RIFF", 4); // [필수] 이 파일이 RIFF 형식임을 명시
    WavData.Append((const uint8*)&ChunkSize, 4); // [필수] 파일의 남은 크기 정보 기록
    WavData.Append((const uint8*)"WAVE", 4); // [필수] WAVE 파일임을 명시

    // --- fmt Chunk 작성 (오디오 포맷 정보) ---
    WavData.Append((const uint8*)"fmt ", 4); // [필수] 포맷 청크 시작 알림 (공백 포함 4글자 주의)
    int32 SubChunk1Size = 16; // [필수] PCM 포맷의 경우 청크 크기는 16으로 고정
    int16 AudioFormat = 1; // [필수] 오디오 포맷 (1 = PCM, 압축되지 않음)
    
    WavData.Append((const uint8*)&SubChunk1Size, 4); // [필수] 위에서 정한 청크 크기 기록
    WavData.Append((const uint8*)&AudioFormat, 2); // [필수] 오디오 포맷 기록
    WavData.Append((const uint8*)&NumChannels, 2); // [필수] 채널 수 기록 (1: Mono, 2: Stereo)
    WavData.Append((const uint8*)&SampleRate, 4); // [필수] 샘플 레이트 기록 (예: 44100, 48000)
    WavData.Append((const uint8*)&ByteRate, 4); // [필수] 바이트 레이트 기록
    WavData.Append((const uint8*)&BlockAlign, 2); // [필수] 블록 정렬 정보 기록
    WavData.Append((const uint8*)&BitsPerSample, 2); // [필수] 비트 깊이 기록

    // --- data Chunk 작성 (실제 오디오 데이터) ---
    WavData.Append((const uint8*)"data", 4); // [필수] 데이터 청크 시작 알림
    WavData.Append((const uint8*)&DataSize, 4); // [필수] 이어질 실제 오디오 데이터의 크기 기록

    // --- PCM Data 병합 ---
    WavData.Append(InPCMData); // [필수] 실제 녹음된 음성 데이터(Body)를 헤더 뒤에 붙임

    return WavData; // [필수] 완성된 WAV 데이터 반환
}

// WAV 데이터를 실제 파일로 디스크에 저장하는 함수
bool UMumulVoiceFunctionLibrary::SaveWavFile(const TArray<uint8>& WavData, FString FileName, FString& OutPath)
{
	if (WavData.Num() == 0) return false; // [선택] 데이터가 비어있으면 저장하지 않음 (안전장치)

	// Saved/BouncedWavs 폴더에 저장
	// [설정] 저장할 폴더 경로 지정 (프로젝트/Saved/RecordedVoice 폴더 사용)
	FString FolderPath = FPaths::ProjectSavedDir() / TEXT("RecordedVoice");
    
	// [필수] 해당 폴더가 없으면 생성 (없으면 저장 실패함)
	IFileManager::Get().MakeDirectory(*FolderPath, true);

	// [선택] 입력받은 파일명에 확장자가 없으면 .wav 자동 추가
	if (!FileName.EndsWith(".wav")) FileName += ".wav";
    
	// [필수] 폴더 경로와 파일명을 합쳐 전체 경로 생성
	OutPath = FolderPath / FileName;

	// [필수] 실제 파일 생성 및 쓰기 수행 (성공 시 true 반환)
	return FFileHelper::SaveArrayToFile(WavData, *OutPath);
}
