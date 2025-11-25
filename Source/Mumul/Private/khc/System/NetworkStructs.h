#pragma once

#include "CoreMinimal.h"
#include "NetworkStructs.generated.h"

USTRUCT()
struct FRequestHeader
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerID;

	UPROPERTY()
	FString Timestamp;
};

// 2. 음성 데이터 전송용 구조체
USTRUCT()
struct FVoiceUploadRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	FString AudioData_Base64; 

	UPROPERTY()
	int32 SampleRate = 0;
};

// 3. 다른 기능용 구조체 예시 (로그인 등)
USTRUCT()
struct FLoginRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString UserID;

	UPROPERTY()
	FString Password;
};