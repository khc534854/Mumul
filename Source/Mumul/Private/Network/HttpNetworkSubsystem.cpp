#include "Network/HttpNetworkSubsystem.h"
#include "HttpModule.h"
#include "Base/MumulGameSettings.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h" // [필수] Base64 인코딩용
#include "Network/NetworkStructs.h"
#include "Misc/DateTime.h"

void UHttpNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("[HTTP] Subsystem Initialized!"));

	const UMumulGameSettings* Settings = GetDefault<UMumulGameSettings>();
	if (Settings)
	{
		BaseURL = Settings->BaseURL;
	}
}

void UHttpNetworkSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UHttpNetworkSubsystem::SendAudioChunk(const TArray<uint8>& WavData, FString MeetingID, FString UserID,
    int32 ChunkIndex, bool bIsLast)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    // 1. URL 설정
    FString FullURL = FString::Printf(TEXT("%s/meeting/%s/audio_chunk"), *BaseURL, *MeetingID);
    Request->SetURL(FullURL);
    
    Request->SetVerb(TEXT("POST"));

    // 2. Boundary 생성
    FString Boundary = TEXT("---------------------------UnrealBoundary12345");
    Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

    // 3. Body 조립
    TArray<uint8> Payload;

    // --- [필드 1] user_id ---
    AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
    AddString(Payload, TEXT("Content-Disposition: form-data; name=\"user_id\"\r\n\r\n"));
    AddString(Payload, UserID);
    AddString(Payload, TEXT("\r\n"));

    // --- [필드 2] chunk_index ---
    AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
    AddString(Payload, TEXT("Content-Disposition: form-data; name=\"chunk_index\"\r\n\r\n"));
    AddString(Payload, FString::FromInt(ChunkIndex));
    AddString(Payload, TEXT("\r\n"));

    // --- [필드 3] upload_timestamp (신규 추가) ---
    int64 CurrentTimestamp = GetCurrentEpochMs(); // 현재 시간(ms) 가져오기
    
    AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
    AddString(Payload, TEXT("Content-Disposition: form-data; name=\"upload_timestamp\"\r\n\r\n"));
    AddString(Payload, FString::Printf(TEXT("%lld"), CurrentTimestamp)); // int64를 문자열로 변환
    AddString(Payload, TEXT("\r\n"));

    // Is Last (주석 유지)
    AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
    AddString(Payload, TEXT("Content-Disposition: form-data; name=\"is_last\"\r\n\r\n"));
    AddString(Payload, bIsLast ? TEXT("true") : TEXT("false")); 
    AddString(Payload, TEXT("\r\n"));

    // --- [필드 4] audio_file (WAV 데이터) ---
    AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
    AddString(Payload, TEXT("Content-Disposition: form-data; name=\"audio_file\"; filename=\"voice.wav\"\r\n"));
    AddString(Payload, TEXT("Content-Type: audio/wav\r\n\r\n"));
    Payload.Append(WavData); 
    AddString(Payload, TEXT("\r\n"));

    // --- [종료] ---
    AddString(Payload, FString::Printf(TEXT("--%s--\r\n"), *Boundary));

    // 4. 전송
    Request->SetContent(Payload);
    Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnSendVoiceComplete);

    // 로그에도 타임스탬프 표시 (확인용)
    UE_LOG(LogTemp, Log, TEXT("[HTTP] Sending Audio Chunk. ID: %s, Idx: %d, Time: %lld, Size: %d"), 
        *MeetingID, ChunkIndex, CurrentTimestamp, Payload.Num());
        
    Request->ProcessRequest();
}

void UHttpNetworkSubsystem::SendLoginRequest(FString ID, FString PW)
{
	// 1. 요청 데이터 생성
	FLoginRequest LoginData;
	LoginData.loginId = ID;
	LoginData.password = PW;

	// 2. JSON 변환
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FLoginRequest::StaticStruct(), &LoginData, JsonString, 0, 0);

	// 3. HTTP 요청 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	FString FullURL = FString::Printf(TEXT("%s/user/login"), *BaseURL); // 엔드포인트: /login

	Request->SetURL(FullURL);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(JsonString);

	// 4. 콜백 연결

	Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnLoginComplete);
	Request->ProcessRequest();
}

void UHttpNetworkSubsystem::StartMeetingRequest(FString MeetingTitle, FString TeamId, int32 OrganizerID, FString Agenda, FString Desc)
{
	FVoiceMeetingStartRequest MeetingStartData;
	MeetingStartData.title = MeetingTitle;
	MeetingStartData.chat_room_id = TeamId;
	MeetingStartData.organizer_id = OrganizerID; // int32 그대로 대입
	MeetingStartData.client_timestamp = GetCurrentEpochMs(); // int64 그대로 대입
	MeetingStartData.agenda = Agenda;
	MeetingStartData.description = Desc;

	SendJsonRequest(
		MeetingStartData,
		TEXT("meeting/start"),
		&UHttpNetworkSubsystem::OnStartMeetingComplete
	);
}

void UHttpNetworkSubsystem::JoinMeetingRequest(int32 UserID, FString MeetingID)
{
	FVoiceMeetingJoinRequest MeetingJoinData;
	MeetingJoinData.user_id = UserID;
	MeetingJoinData.client_timestamp = GetCurrentEpochMs();

	FString Endpoint = FString::Printf(TEXT("meeting/%s/join"), *MeetingID);

	SendJsonRequest(
		MeetingJoinData,
		Endpoint,
		&UHttpNetworkSubsystem::OnJoinMeetingComplete
	);
}

void UHttpNetworkSubsystem::EndMeetingRequest(FString MeetingID)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	// URL 설정: /meeting/{meeting_id}/end
	FString FullURL = FString::Printf(TEXT("%s/meeting/%s/end"), *BaseURL, *MeetingID);

	Request->SetURL(FullURL);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");

	// Body는 필요 없음 
	Request->SetContentAsString(TEXT("{}"));

    Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnEndMeetingComplete);
    
    UE_LOG(LogTemp, Warning, TEXT("[HTTP] >> Requesting End Meeting API... ID: %s"), *MeetingID);
    Request->ProcessRequest();
}

void UHttpNetworkSubsystem::OnLoginComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		OnLoginResponse.Broadcast(false, TEXT("네트워크 연결 실패"));
		return;
	}

	int32 Code = Response->GetResponseCode();
	FString Content = Response->GetContentAsString();

	if (Code == 200) // 성공
	{
		OnLoginResponse.Broadcast(true, Content);
	}
	else if (Code == 401) // 실패
	{
		FFailResponse FailData;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &FailData, 0, 0))
		{
			OnLoginResponse.Broadcast(false, FailData.detail.message);
		}
		else
		{
			OnLoginResponse.Broadcast(false, TEXT("로그인 실패 (알 수 없는 오류)"));
		}
	}
	else
	{
		OnLoginResponse.Broadcast(false, FString::Printf(TEXT("서버 오류: %d"), Code));
	}
}

void UHttpNetworkSubsystem::OnStartMeetingComplete(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                   bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		int32 Code = Response->GetResponseCode();
		FString Content = Response->GetContentAsString();

		if (Code == 200)
		{
			FVoiceMeetingStartSuccessResponse SuccessData;
			if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &SuccessData, 0, 0))
			{
				UE_LOG(LogTemp, Log, TEXT("[HTTP] Meeting Started: %s (Status: %s)"), *SuccessData.meeting_id,
				       *SuccessData.status);
				OnStartMeeting.Broadcast(true, SuccessData.meeting_id);
				return;
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[HTTP] Start Meeting Failed: %d / %s"), Code, *Content);
		}
	}

	OnStartMeeting.Broadcast(false, TEXT(""));
}

void UHttpNetworkSubsystem::OnJoinMeetingComplete(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                  bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		int32 Code = Response->GetResponseCode();
		FString Content = Response->GetContentAsString();

		if (Code == 200)
		{
			FVoiceMeetingJoinSuccessResponse SuccessData;
			if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &SuccessData, 0, 0))
			{
				UE_LOG(LogTemp, Log, TEXT("[HTTP] Joined Meeting: %s (User: %d)"), *SuccessData.meeting_id,
				       SuccessData.user_id);
				OnJoinMeeting.Broadcast(true);
				return;
			}
		}
		else // 404, 409, 500 등
		{
			// 에러 메시지 파싱 (공통 에러 구조체 사용)
			// {"detail": "..."} 형태라면 FLoginFailResponse와 같은 구조체 재사용 가능
			UE_LOG(LogTemp, Error, TEXT("[HTTP] Join Meeting Failed: %d / %s"), Code, *Content);
		}
	}

	OnJoinMeeting.Broadcast(false);
}

void UHttpNetworkSubsystem::OnEndMeetingComplete(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                 bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		int32 Code = Response->GetResponseCode();
		FString Content = Response->GetContentAsString();

        if (Code == 200)
        {
            FVoiceMeetingEndResponse EndData;
            if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &EndData, 0, 0))
            {
                UE_LOG(LogTemp, Warning, TEXT("[HTTP] << End Meeting SUCCESS! ID: %s, Duration: %lld ms, Participants: %d"), 
                    *EndData.meeting_id, EndData.duration_ms, EndData.participant_count);
                OnEndMeeting.Broadcast(true);
                return;
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[HTTP] << End Meeting FAILED! Code: %d, Error: %s"), Code, *Content);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[HTTP] << End Meeting CONNECTION FAILED!"));
    }
    
    OnEndMeeting.Broadcast(false);
}

int64 UHttpNetworkSubsystem::GetCurrentEpochMs()
{
	const FDateTime EpochOrigin(1970, 1, 1);

	// 2. 현재 UTC 시간 가져오기
	FDateTime CurrentTime = FDateTime::UtcNow();

	// 3. 차이(TimeSpan) 계산
	FTimespan Timespan = CurrentTime - EpochOrigin;

	// 4. 전체 밀리초로 변환 (double -> int64 캐스팅)
	int64 EpochMs = static_cast<int64>(Timespan.GetTotalMilliseconds());

	return EpochMs;
}

void UHttpNetworkSubsystem::SendChatHistoryRequest(int32 UserID)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

	// URL: /learning_chatbot/history/{userId}/{sessionId}
	// 명세서 요청: sessionId를 userId와 동일하게 전송
	FString FullURL = FString::Printf(TEXT("%s/learning_chatbot/history/%d/%d"), *BaseURL, UserID, UserID);
    
	Request->SetURL(FullURL);
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "application/json");

	Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnChatHistoryComplete);
	Request->ProcessRequest();
    
	UE_LOG(LogTemp, Log, TEXT("[HTTP] Request Chat History: %s"), *FullURL);
}

void UHttpNetworkSubsystem::OnChatHistoryComplete(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		OnChatHistoryResponse.Broadcast(false, TEXT("네트워크 연결 실패"));
		return;
	}

	int32 Code = Response->GetResponseCode();
	FString Content = Response->GetContentAsString();

	if (Code == 200)
	{
		// 성공 시 JSON 원본 전달
		OnChatHistoryResponse.Broadcast(true, Content);
	}
	else
	{
		// 실패 시 에러 메시지 전달 (404 등)
		OnChatHistoryResponse.Broadcast(false, Content);
	}
}

void UHttpNetworkSubsystem::OnSendVoiceComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (bWasSuccessful && Response.IsValid())
	{
		// 성공적으로 응답을 받음
		int32 ResponseCode = Response->GetResponseCode();
		FString Content = Response->GetContentAsString();

		if (ResponseCode >= 200 && ResponseCode < 300)
		{
			FVoiceChunkResponse ResponseData;

            if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &ResponseData, 0, 0))
            {
                // 3. 성공! 데이터 사용
                UE_LOG(LogTemp, Warning, TEXT("[HTTP] << Audio Chunk Upload SUCCESS! Meeting: %s, Chunk: %d"), 
                    *ResponseData.meeting_id, ResponseData.chunk_index);
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[HTTP] << Audio Chunk Upload FAILED! Code: %d, Error: %s"), ResponseCode, *Content);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[HTTP] << Audio Chunk Upload FAILED! Code: %d, Error: %s"), ResponseCode, *Content);
        }
    }
    else
    {
        // 네트워크 연결 실패 등
        UE_LOG(LogTemp, Error, TEXT("[HTTP] << Audio Chunk Upload CONNECTION FAILED!"));
    }

    OnSendVoiceCompleteDelegate_LowLevel.Broadcast(Request, Response, bWasSuccessful);
}

void UHttpNetworkSubsystem::AddString(TArray<uint8>& OutPayload, const FString& InString)
{
	FTCHARToUTF8 Converter(*InString);
	OutPayload.Append((uint8*)Converter.Get(), Converter.Length());
}


void UHttpNetworkSubsystem::SendTeamChatListRequest(int32 UserID)
{
	// 3. HTTP 요청 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	FString FullURL = FString::Printf(TEXT("%s/chat/rooms?userId=%d"), *BaseURL, UserID);
	UE_LOG(LogTemp, Warning, TEXT("%s/chat/rooms?userId=%d"), *BaseURL, UserID);

	Request->SetURL(FullURL);
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "application/json");

	// 4. 콜백 연결

	Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnTeamChatListComplete);
	Request->ProcessRequest();
}

void UHttpNetworkSubsystem::OnTeamChatListComplete(FHttpRequestPtr Request, FHttpResponsePtr Response,
                                                   bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		OnTeamChatListResponse.Broadcast(false, TEXT("네트워크 연결 실패"));
		return;
	}

	int32 Code = Response->GetResponseCode();
	FString Content = Response->GetContentAsString();

	if (Code == 200) // 성공
	{
		// [수정] 성공 시에는 가공하지 말고 JSON 원본(Content)을 그대로 보냅니다.
		// 그래야 위젯에서 데이터를 뽑아 쓸 수 있습니다.
		OnTeamChatListResponse.Broadcast(true, Content);
	}
	else if (Code == 404) // 실패
	{
		FFailResponse FailData;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &FailData, 0, 0))
		{
			OnTeamChatListResponse.Broadcast(false, FailData.detail.message);
		}
		else
		{
			OnTeamChatListResponse.Broadcast(false, TEXT("팀채팅 불러오기 실패 (알 수 없는 오류)"));
		}
	}
	else
	{
		OnTeamChatListResponse.Broadcast(false, FString::Printf(TEXT("서버 오류: %d"), Code));
	}
}


void UHttpNetworkSubsystem::SendTeamChatMessageRequest(const FString& TeamChatID)
{
	// 1. 요청 데이터 생성 없음

	// 2. JSON 변환 없음

	// 3. HTTP 요청 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	FString FullURL = FString::Printf(TEXT("%s/chat/rooms/%s/messages"), *BaseURL, *TeamChatID);
	UE_LOG(LogTemp, Warning, TEXT("%s/chat/rooms/%s/messages"), *BaseURL, *TeamChatID);

	Request->SetURL(FullURL);
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "application/json");

	// 4. 콜백 연결

	Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnTeamChatMessageComplete);
	Request->ProcessRequest();
}

void UHttpNetworkSubsystem::OnTeamChatMessageComplete(TSharedPtr<IHttpRequest> HttpRequest,
                                                      TSharedPtr<IHttpResponse> HttpResponse, bool bArg)
{
	if (!bArg || !HttpResponse.IsValid())
	{
		OnTeamChatMessageResponse.Broadcast(false, TEXT("네트워크 연결 실패"));
		return;
	}

	int32 Code = HttpResponse->GetResponseCode();
	FString Content = HttpResponse->GetContentAsString();

	if (Code == 200) // 성공
	{
		// [수정] 성공 시에는 가공하지 말고 JSON 원본(Content)을 그대로 보냅니다.
		// 그래야 위젯에서 데이터를 뽑아 쓸 수 있습니다.
		OnTeamChatMessageResponse.Broadcast(true, Content);
	}
	else if (Code == 404) // 실패
	{
		FFailResponse FailData;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &FailData, 0, 0))
		{
			OnTeamChatMessageResponse.Broadcast(false, FailData.detail.message);
		}
		else
		{
			OnTeamChatMessageResponse.Broadcast(false, TEXT("채팅 저장 실패 (알 수 없는 오류)"));
		}
	}
	else
	{
		OnTeamChatMessageResponse.Broadcast(false, FString::Printf(TEXT("서버 오류: %d"), Code));
	}
}


void UHttpNetworkSubsystem::SendChatMessageRequest(const FString& TeamChatID, const int32& UserID,
                                                   const FString& Message, const FString& Time)
{
	// 1. 요청 데이터 생성
	FChatMessageRequest ChatMessage;
	ChatMessage.userId = UserID;
	ChatMessage.message = Message;
	ChatMessage.createdAt = Time;

	// 2. JSON 변환
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FChatMessageRequest::StaticStruct(), &ChatMessage, JsonString, 0,
	                                                0);

	// 3. HTTP 요청 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	FString FullURL = FString::Printf(TEXT("%s/chat/rooms/%s/messages"), *BaseURL, *TeamChatID);
	UE_LOG(LogTemp, Warning, TEXT("%s/chat/rooms/%s/messages"), *BaseURL, *TeamChatID)

	Request->SetURL(FullURL);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(JsonString);

	// 4. 콜백 연결

	Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnChatMessageComplete);
	Request->ProcessRequest();
}


void UHttpNetworkSubsystem::OnChatMessageComplete(TSharedPtr<IHttpRequest> HttpRequest,
                                                  TSharedPtr<IHttpResponse> HttpResponse, bool bArg) const
{
	if (!bArg || !HttpResponse.IsValid())
	{
		OnChatMessageResponse.Broadcast(false, TEXT("네트워크 연결 실패"));
		return;
	}

	int32 Code = HttpResponse->GetResponseCode();
	FString Content = HttpResponse->GetContentAsString();

	if (Code == 200) // 성공
	{
		// [수정] 성공 시에는 가공하지 말고 JSON 원본(Content)을 그대로 보냅니다.
		// 그래야 위젯에서 데이터를 뽑아 쓸 수 있습니다.
		OnChatMessageResponse.Broadcast(true, TEXT("채팅 저장 성공"));
	}
	else if (Code == 404) // 실패
	{
		FFailResponse FailData;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &FailData, 0, 0))
		{
			OnChatMessageResponse.Broadcast(false, FailData.detail.message);
		}
		else
		{
			OnChatMessageResponse.Broadcast(false, TEXT("채팅 저장 실패 (알 수 없는 오류)"));
		}
	}
	else
	{
		OnChatMessageResponse.Broadcast(false, FString::Printf(TEXT("서버 오류: %d"), Code));
	}
}


void UHttpNetworkSubsystem::SendCreateTeamChatRequest(const FString& TeamName, const TArray<int32>& UserIDs)
{
	// 1. 요청 데이터 생성
	FCreateTeamChatRequest CreateTeamChat;
	CreateTeamChat.groupName = TeamName;
	CreateTeamChat.userIdList = UserIDs;

	// 2. JSON 변환
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FCreateTeamChatRequest::StaticStruct(), &CreateTeamChat, JsonString,
	                                                0, 0);

	// 3. HTTP 요청 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	FString FullURL = FString::Printf(TEXT("%s/chat/rooms"), *BaseURL);
	UE_LOG(LogTemp, Warning, TEXT("%s/chat/rooms"), *BaseURL)

	Request->SetURL(FullURL);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(JsonString);

	// 4. 콜백 연결

	Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnCreateTeamChatComplete);
	Request->ProcessRequest();
}

void UHttpNetworkSubsystem::OnCreateTeamChatComplete(TSharedPtr<IHttpRequest> HttpRequest,
                                                     TSharedPtr<IHttpResponse> HttpResponse, bool bArg) const
{
	if (!bArg || !HttpResponse.IsValid())
	{
		OnCreateTeamChatResponse.Broadcast(false, TEXT("네트워크 연결 실패"));
		return;
	}

	int32 Code = HttpResponse->GetResponseCode();
	FString Content = HttpResponse->GetContentAsString();

	if (Code == 200) // 성공
	{
		// [수정] 성공 시에는 가공하지 말고 JSON 원본(Content)을 그대로 보냅니다.
		// 그래야 위젯에서 데이터를 뽑아 쓸 수 있습니다.
		OnCreateTeamChatResponse.Broadcast(true, Content);
	}
	else if (Code == 404) // 실패
	{
		FFailResponse FailData;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &FailData, 0, 0))
		{
			OnCreateTeamChatResponse.Broadcast(false, FailData.detail.message);
		}
		else
		{
			OnCreateTeamChatResponse.Broadcast(false, TEXT("팀채팅 생성 실패 (알 수 없는 오류)"));
		}
	}
	else
	{
		OnCreateTeamChatResponse.Broadcast(false, FString::Printf(TEXT("서버 오류: %d"), Code));
	}
}

void UHttpNetworkSubsystem::OnLearningQuizComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		OnCreateTeamChatResponse.Broadcast(false, TEXT("네트워크 연결 실패"));
		return;
	}

	int32 Code = Response->GetResponseCode();
	FString Content = Response->GetContentAsString();

	if (Code == 200) // 성공
	{
		// [수정] 성공 시에는 가공하지 말고 JSON 원본(Content)을 그대로 보냅니다.
		// 그래야 위젯에서 데이터를 뽑아 쓸 수 있습니다.
		OnCreateTeamChatResponse.Broadcast(true, Content);
	}
	else if (Code == 400) // 실패
	{
		FFailResponse FailData;
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &FailData, 0, 0))
		{
			OnCreateTeamChatResponse.Broadcast(false, FailData.detail.message);
		}
		else
		{
			OnCreateTeamChatResponse.Broadcast(false, TEXT("학습퀴즈 요청 실패 (알 수 없는 오류)"));
		}
	}
	else
	{
		OnCreateTeamChatResponse.Broadcast(false, FString::Printf(TEXT("서버 오류: %d"), Code));
	}
}
