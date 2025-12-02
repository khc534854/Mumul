#include "HttpNetworkSubsystem.h"
#include "HttpModule.h"
#include "MumulGameSettings.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h" // [필수] Base64 인코딩용
#include "khc/System/NetworkStructs.h"

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
    int32 ChunkIndex)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    // 1. URL 설정 (파이썬 요구사항: meetings/{meeting_id}/audio_chunk)
    // BaseURL 뒤에 슬래시가 없다고 가정하고 포맷팅
    FString FullURL = FString::Printf(TEXT("%s/meeting/%s/audio_chunk"), *BaseURL, *MeetingID);
    Request->SetURL(FullURL);
    
    Request->SetVerb(TEXT("POST"));

    // 2. Boundary 생성
    FString Boundary = TEXT("---------------------------UnrealBoundary12345");
    Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));

    // 3. Body 조립 (순서대로 샌드위치 만들기)
    TArray<uint8> Payload;

    // --- [필드 1] user_id ---
    AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
    AddString(Payload, TEXT("Content-Disposition: form-data; name=\"user_id\"\r\n\r\n"));
    AddString(Payload, UserID);
    AddString(Payload, TEXT("\r\n"));

    // --- [필드 2] chunk_index ---
    AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
    AddString(Payload, TEXT("Content-Disposition: form-data; name=\"chunk_index\"\r\n\r\n"));
    AddString(Payload, FString::FromInt(ChunkIndex)); // 숫자를 문자열로 변환해서 전송
    AddString(Payload, TEXT("\r\n"));

    // Is Last
    //AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
    //AddString(Payload, TEXT("Content-Disposition: form-data; name=\"is_last\"\r\n\r\n"));
    //AddString(Payload, bIsLast ? TEXT("true") : TEXT("false")); // 파이썬이 문자열 "true"/"false"로 받음
    //AddString(Payload, TEXT("\r\n"));

    // --- [필드 3] audio_file (WAV 데이터) ---
    AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
    // 서버가 요구한 이름: name="audio_file"
    AddString(Payload, TEXT("Content-Disposition: form-data; name=\"audio_file\"; filename=\"voice.wav\"\r\n"));
    AddString(Payload, TEXT("Content-Type: audio/wav\r\n\r\n"));
    Payload.Append(WavData); // 바이너리 데이터 추가
    AddString(Payload, TEXT("\r\n"));

    // --- [종료] ---
    AddString(Payload, FString::Printf(TEXT("--%s--\r\n"), *Boundary));

    // 4. 전송
    Request->SetContent(Payload);
    Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnSendVoiceComplete);

    UE_LOG(LogTemp, Log, TEXT("[HTTP] Sending Audio Chunk to: %s (Size: %d bytes)"), *FullURL, Payload.Num());
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
        // [수정] 성공 시에는 가공하지 말고 JSON 원본(Content)을 그대로 보냅니다.
        // 그래야 위젯에서 데이터를 뽑아 쓸 수 있습니다.
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
                UE_LOG(LogTemp, Log, TEXT("[HTTP] Upload Success! Meeting: %s, Chunk: %d"), 
                    *ResponseData.meeting_id, ResponseData.chunk_index);
                
                // (선택 사항) 여기서 델리게이트를 호출해 UI나 다른 곳에 알릴 수도 있습니다.
                // OnUploadSuccess.Broadcast(ResponseData); 
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[HTTP] JSON Parsing Failed! Content: %s"), *Content);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[HTTP] Server Error. Code: %d, Content: %s"), ResponseCode, *Content);
        }
    }
    else
    {
        // 네트워크 연결 실패 등
        UE_LOG(LogTemp, Error, TEXT("[HTTP] Connection Failed!"));
    }
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


void UHttpNetworkSubsystem::SendChatMessageRequest(const FString& TeamChatID, const int32& UserID, const FString& Message, const FString& Time)
{
    // 1. 요청 데이터 생성
    FChatMessageRequest ChatMessage;
    ChatMessage.userId = UserID;
    ChatMessage.message = Message;
    ChatMessage.createdAt = Time;
    
    // 2. JSON 변환
    FString JsonString;
    FJsonObjectConverter::UStructToJsonObjectString(FChatMessageRequest::StaticStruct(), &ChatMessage, JsonString, 0, 0);

    // 3. HTTP 요청 생성
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    FString FullURL = FString::Printf(TEXT("%s/chat/rooms/%s/messages"), *BaseURL, *TeamChatID);
    
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
    FJsonObjectConverter::UStructToJsonObjectString(FCreateTeamChatRequest::StaticStruct(), &CreateTeamChat, JsonString, 0, 0);

    // 3. HTTP 요청 생성
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    FString FullURL = FString::Printf(TEXT("%s/chat/rooms"), *BaseURL);
    
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