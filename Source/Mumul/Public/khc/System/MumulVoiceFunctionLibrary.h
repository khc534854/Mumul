// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MumulVoiceFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UMumulVoiceFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// PCM 데이터를 WAV 바이너리로 변환
	static TArray<uint8> ConvertPCMToWAV(const TArray<uint8>& InPCMData, int32 SampleRate, int32 NumChannels);

	// WAV 바이너리를 파일로 저장
	UFUNCTION(BlueprintCallable, Category = "Voice|File")
	static bool SaveWavFile(const TArray<uint8>& WavData, FString FileName, FString& OutPath);
};
