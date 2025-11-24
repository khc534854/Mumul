// Fill out your copyright notice in the Description page of Project Settings.


#include "khc/System/MumulVoiceFunctionLibrary.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

TArray<uint8> UMumulVoiceFunctionLibrary::ConvertPCMToWAV(const TArray<uint8>& InPCMData, int32 SampleRate,
	int32 NumChannels)
{
	TArray<uint8> WavData;
	int32 BitsPerSample = 16; // 보통 16비트 사용
	int32 DataSize = InPCMData.Num();
	int32 ChunkSize = 36 + DataSize;
	int32 ByteRate = SampleRate * NumChannels * (BitsPerSample / 8);
	int32 BlockAlign = NumChannels * (BitsPerSample / 8);

	// RIFF Chunk
	WavData.Append((const uint8*)"RIFF", 4);
	WavData.Append((const uint8*)&ChunkSize, 4);
	WavData.Append((const uint8*)"WAVE", 4);

	// fmt Chunk
	WavData.Append((const uint8*)"fmt ", 4);
	int32 SubChunk1Size = 16;
	int16 AudioFormat = 1; // PCM
	WavData.Append((const uint8*)&SubChunk1Size, 4);
	WavData.Append((const uint8*)&AudioFormat, 2);
	WavData.Append((const uint8*)&NumChannels, 2); // 16비트 변환 주의
	WavData.Append((const uint8*)&SampleRate, 4);
	WavData.Append((const uint8*)&ByteRate, 4);
	WavData.Append((const uint8*)&BlockAlign, 2);
	WavData.Append((const uint8*)&BitsPerSample, 2);

	// data Chunk
	WavData.Append((const uint8*)"data", 4);
	WavData.Append((const uint8*)&DataSize, 4);

	// PCM Data
	WavData.Append(InPCMData);

	return WavData;
}

bool UMumulVoiceFunctionLibrary::SaveWavFile(const TArray<uint8>& WavData, FString FileName, FString& OutPath)
{
	if (WavData.Num() == 0) return false;

	// Saved/BouncedWavs 폴더에 저장
	FString FolderPath = FPaths::ProjectSavedDir() / TEXT("RecordedVoice");
	IFileManager::Get().MakeDirectory(*FolderPath, true);

	if (!FileName.EndsWith(".wav")) FileName += ".wav";
	OutPath = FolderPath / FileName;

	return FFileHelper::SaveArrayToFile(WavData, *OutPath);
}
