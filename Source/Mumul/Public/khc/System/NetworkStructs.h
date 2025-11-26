#pragma once

#include "CoreMinimal.h"
#include "NetworkStructs.generated.h"

USTRUCT()
struct FPlayerLogRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerID;

	UPROPERTY()
	FString Timestamp; // 시간은 보통 문자열로 보냅니다.
};

USTRUCT()
struct FRequestHeader
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerID;

	UPROPERTY()
	FString Timestamp;
};

USTRUCT()
struct FVoiceMetadata
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	FString RoomID;

	UPROPERTY()
	int32 TeamID = 0; // 추가된 정보
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

USTRUCT()
struct FVoiceChunkResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString meeting_id;

	UPROPERTY()
	FString user_id;

	UPROPERTY()
	int32 chunk_index = 0;
};