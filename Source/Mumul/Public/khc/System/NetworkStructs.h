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

USTRUCT()
struct FLoginResponse
{
	GENERATED_BODY()

	UPROPERTY()
	bool success = false; // 로그인 성공 여부

	UPROPERTY()
	FString message;      // (선택) 실패 사유 등 메시지
};

USTRUCT()
struct FLoginRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString loginId;

	UPROPERTY()
	FString password;
};

// [응답] 로그인 성공 (200 OK)
USTRUCT()
struct FLoginSuccessResponse
{
	GENERATED_BODY()

	UPROPERTY()
	int32 userId = 0;

	UPROPERTY()
	FString name;

	UPROPERTY()
	int32 campId = 0;

	UPROPERTY()
	bool tendencyCompleted = false;
};

// [응답] 로그인 실패 (401 Error) - 내부 detail 구조체 필요
USTRUCT()
struct FErrorDetail
{
	GENERATED_BODY()

	UPROPERTY()
	FString errorCode;

	UPROPERTY()
	FString message;
};

USTRUCT()
struct FLoginFailResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FErrorDetail detail;
};

USTRUCT()
struct FMeetingSummaryResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString type; // "summary_complete" 등 메시지 타입 구분

	UPROPERTY()
	FString meeting_id;

	UPROPERTY()
	FString summary_text; // 요약된 내용
};