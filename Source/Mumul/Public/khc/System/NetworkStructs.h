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
struct FVoiceMeetingStartRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString title;

	UPROPERTY()
	int32 organizer_id = 0; // integer

	UPROPERTY()
	int64 client_timestamp = 0; // integer

	UPROPERTY()
	FString agenda; 

	UPROPERTY()
	FString description;
};

USTRUCT()
struct FVoiceMeetingStartSuccessResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString meeting_id;

	UPROPERTY()
	FString status;
};

USTRUCT()
struct FVoiceMeetingJoinRequest
{
	GENERATED_BODY()

	UPROPERTY()
	int32 user_id = 0; // integer

	UPROPERTY()
	int64 client_timestamp = 0; // integer
};

USTRUCT()
struct FVoiceMeetingJoinSuccessResponse
{
	GENERATED_BODY()

	UPROPERTY()
	int32 participant_id = 0;

	UPROPERTY()
	FString meeting_id; 
    
	UPROPERTY()
	int32 user_id = 0;
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
struct FVoiceMeetingEndResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString meeting_id;

	UPROPERTY()
	FString status;

	UPROPERTY()
	int64 duration_ms = 0;

	UPROPERTY()
	int32 participant_count = 0;

	UPROPERTY()
	int32 total_segments = 0;
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

	UPROPERTY()
	FString userType;
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
struct FFailResponse
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

// NetworkStructs.h 에 추가

// --- [WebSocket 요청용 구조체] ---

// 1. 세션 시작 (start_chat)
USTRUCT()
struct FWSRequest_StartChat
{
	GENERATED_BODY()

	UPROPERTY()
	FString event = TEXT("start_chat");

	UPROPERTY()
	int32 sessionId; // 명세서: 유저아이디와 동일

	UPROPERTY()
	int32 userId;
};

// 2. 질문 보내기 (query)
USTRUCT()
struct FWSRequest_Query
{
	GENERATED_BODY()

	UPROPERTY()
	FString event = TEXT("query");

	UPROPERTY()
	int32 userId; // 유저아이디

	UPROPERTY()
	int32 sessionId; // 유저아이디와 동일

	UPROPERTY()
	FString query;
};

// 3. 세션 종료 (end_chat)
USTRUCT()
struct FWSRequest_EndChat
{
	GENERATED_BODY()

	UPROPERTY()
	FString event = TEXT("end_chat");

	UPROPERTY()
	int32 sessionId; // 유저아이디와 동일
};


// --- [WebSocket 응답 데이터용 구조체] ---

// 답변 수신용 (answer)
USTRUCT()
struct FWSResponse_Answer
{
	GENERATED_BODY()

	UPROPERTY()
	FString event;

	UPROPERTY()
	int32 sessionId;
    
	UPROPERTY()
	int32 userId; // [추가됨] 명세서 반영

	UPROPERTY()
	FString answer;
};

USTRUCT()
struct FUserDetail
{
	GENERATED_BODY()

	UPROPERTY()
	int32 userId;

	UPROPERTY()
	FString userName;
};

// [응답] 팀채팅리스트 성공 (200 OK)
USTRUCT()
struct FTeamChatListResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString teamChatId;

	UPROPERTY()
	FString teamName;

	UPROPERTY()
	TArray<FUserDetail> users;
};

// TeamChatMessage Structure
USTRUCT()
struct FTeamChatMessageResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString chatId;
	
	UPROPERTY()
	int32 userId;
	
	UPROPERTY()
	FString userName;
	
	UPROPERTY()
	FString message;
	
	UPROPERTY()
	FString createdAt;
};


// ChatMessage Structure
USTRUCT()
struct FChatMessageRequest
{
	GENERATED_BODY()

	UPROPERTY()
	int32 userId;
	
	UPROPERTY()
	FString message;
	
	UPROPERTY()
	FString createdAt;
};

// CreatTeamChat Structure
USTRUCT()
struct FCreateTeamChatRequest
{
	GENERATED_BODY()

	UPROPERTY()
	FString groupName;
	
	UPROPERTY()
	TArray<int32> userIdList;
};

USTRUCT()
struct FCreateTeamChatResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FString groupId;
	
	UPROPERTY()
	FString groupName;
	
	UPROPERTY()
	TArray<int32> userIdList;
};

USTRUCT()
struct FChatHistoryMessage
{
	GENERATED_BODY()

	UPROPERTY()
	FString role; // "user" or "assistant"

	UPROPERTY()
	FString content;

	UPROPERTY()
	FString created_at;
};

USTRUCT()
struct FChatHistoryResponse
{
	GENERATED_BODY()

	UPROPERTY()
	int32 sessionId = 0;

	UPROPERTY()
	int32 userId = 0;

	UPROPERTY()
	TArray<FChatHistoryMessage> messages;
};