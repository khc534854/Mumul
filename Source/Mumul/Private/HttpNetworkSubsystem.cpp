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
        FLoginSuccessResponse SuccessData;
        if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &SuccessData, 0, 0))
        {
            // 성공 메시지로 이름 전달 (예: "홍길동님 환영합니다")
            FString Msg = FString::Printf(TEXT("%s님 환영합니다."), *SuccessData.name);
            OnLoginResponse.Broadcast(true, Msg);
        }
        else
        {
            OnLoginResponse.Broadcast(false, TEXT("응답 데이터 파싱 실패"));
        }
    }
    else if (Code == 401) // 실패 (아이디/비번 틀림)
    {
        FLoginFailResponse FailData;
        if (FJsonObjectConverter::JsonObjectStringToUStruct(Content, &FailData, 0, 0))
        {
            // 서버가 보낸 에러 메시지 그대로 전달
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
