#include "HttpNetworkSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h" // [필수] Base64 인코딩용
#include "khc/System/NetworkStructs.h"

void UHttpNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[HTTP] Subsystem Initialized!"));
}

void UHttpNetworkSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

// void UHttpNetworkSubsystem::SendVoiceDataToPython(const TArray<uint8>& WavData)
// {
//     FVoiceUploadRequest VoiceRequest;
//     
//     // 데이터를 채웁니다.
//     VoiceRequest.PlayerName = TEXT("Player1"); // 나중에 실제 이름으로
//     VoiceRequest.SampleRate = 48000; // 메타데이터 필요시
//
//     // [핵심] 2. WAV 바이너리 -> Base64 문자열 변환
//     // TArray<uint8>을 FString으로 인코딩합니다.
//     VoiceRequest.AudioData_Base64 = FBase64::Encode(WavData);
//
//     // 3. 템플릿 함수 호출 (알아서 JSON으로 바꿔서 쏨)
//     SendJsonRequest<FVoiceUploadRequest>(VoiceRequest, TEXT("upload-voice-json"));
// }

// void UHttpNetworkSubsystem::SendMultipartVoice(const TArray<uint8>& WavData, const FString& MetaJsonString)
// {
//     TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
//     
//     // URL 설정 (BaseURL + 엔드포인트)
//     FString FullURL = FString::Printf(TEXT("%s/upload-multipart"), *BaseURL);
//     Request->SetURL(FullURL);
//     Request->SetVerb(TEXT("POST"));
//
//     // Boundary 설정
//     FString Boundary = TEXT("---------------------------UnrealBoundaryHere");
//     Request->SetHeader(TEXT("Content-Type"), FString::Printf(TEXT("multipart/form-data; boundary=%s"), *Boundary));
//
//     // Body 조립
//     TArray<uint8> Payload;
//
//     // 1. 메타데이터 (JSON)
//     AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
//     AddString(Payload, TEXT("Content-Disposition: form-data; name=\"metadata\"\r\n\r\n"));
//     AddString(Payload, MetaJsonString);
//     AddString(Payload, TEXT("\r\n"));
//
//     // 2. 파일 데이터 (WAV)
//     AddString(Payload, FString::Printf(TEXT("--%s\r\n"), *Boundary));
//     AddString(Payload, TEXT("Content-Disposition: form-data; name=\"file\"; filename=\"voice.wav\"\r\n"));
//     AddString(Payload, TEXT("Content-Type: audio/wav\r\n\r\n"));
//     Payload.Append(WavData); // 바이너리 그대로 추가
//     AddString(Payload, TEXT("\r\n"));
//
//     // 3. 종료 경계선
//     AddString(Payload, FString::Printf(TEXT("--%s--\r\n"), *Boundary));
//
//     Request->SetContent(Payload);
//     Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnSendVoiceComplete);
//
//     UE_LOG(LogTemp, Log, TEXT("[HTTP] Sending Multipart Request to: %s (Size: %d bytes)"), *FullURL, Payload.Num());
//     Request->ProcessRequest();
// }

void UHttpNetworkSubsystem::SendAudioChunk(const TArray<uint8>& WavData, FString MeetingID, FString UserID,
    int32 ChunkIndex)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();

    // 1. URL 설정 (파이썬 요구사항: meetings/{meeting_id}/audio_chunk)
    // BaseURL 뒤에 슬래시가 없다고 가정하고 포맷팅
    FString FullURL = FString::Printf(TEXT("%s/meetings/%s/audio_chunk"), *BaseURL, *MeetingID);
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
