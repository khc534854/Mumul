#include "HttpNetworkSubsystem.h"
#include "HttpModule.h"
#include "MumulGameSettings.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h" // [필수] Base64 인코딩용
#include "khc/System/NetworkStructs.h"
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

void UHttpNetworkSubsystem::StartMeetingRequest(FString MeetingTitle, int32 OrganizerID, FString Agenda, FString Desc)
{
    FVoiceMeetingStartRequest MeetingStartData;
    MeetingStartData.title = MeetingTitle;
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
    
    UE_LOG(LogTemp, Log, TEXT("[HTTP] Request End Meeting: %s"), *MeetingID);
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
        FLoginFailResponse FailData;
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
                UE_LOG(LogTemp, Log, TEXT("[HTTP] Meeting Started: %s (Status: %s)"), *SuccessData.meeting_id, *SuccessData.status);
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
                UE_LOG(LogTemp, Log, TEXT("[HTTP] Joined Meeting: %s (User: %d)"), *SuccessData.meeting_id, SuccessData.user_id);
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
                UE_LOG(LogTemp, Log, TEXT("[HTTP] Meeting Ended! ID: %s, Duration: %lld ms"), *EndData.meeting_id, EndData.duration_ms);
                OnEndMeeting.Broadcast(true);
                return;
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[HTTP] End Meeting Failed: %d / %s"), Code, *Content);
        }
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

                OnSendVoiceCompleteDelegate_LowLevel.Broadcast(Request, Response, bWasSuccessful);
                
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

    OnSendVoiceCompleteDelegate_LowLevel.Broadcast(Request, Response, bWasSuccessful);
}

void UHttpNetworkSubsystem::AddString(TArray<uint8>& OutPayload, const FString& InString)
{
    FTCHARToUTF8 Converter(*InString);
    OutPayload.Append((uint8*)Converter.Get(), Converter.Length());
}
