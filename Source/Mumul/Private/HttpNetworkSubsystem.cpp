#include "HttpNetworkSubsystem.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Misc/Base64.h" // [필수] Base64 인코딩용
#include "khc/System/NetworkStructs.h"

void UHttpNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("HttpNetworkSubsystem Initialized!"));
}

void UHttpNetworkSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UHttpNetworkSubsystem::SendVoiceDataToPython(const TArray<uint8>& WavData)
{
    FVoiceUploadRequest VoiceRequest;
    
    // 데이터를 채웁니다.
    VoiceRequest.PlayerName = TEXT("Player1"); // 나중에 실제 이름으로
    VoiceRequest.SampleRate = 48000; // 메타데이터 필요시

    // [핵심] 2. WAV 바이너리 -> Base64 문자열 변환
    // TArray<uint8>을 FString으로 인코딩합니다.
    VoiceRequest.AudioData_Base64 = FBase64::Encode(WavData);

    // 3. 템플릿 함수 호출 (알아서 JSON으로 바꿔서 쏨)
    SendJsonRequest<FVoiceUploadRequest>(VoiceRequest, TEXT("upload-voice-json"));
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
            UE_LOG(LogTemp, Log, TEXT("[HTTP] Upload Success! Code: %d, Response: %s"), ResponseCode, *Content);
            // 여기서 파이썬이 보내준 결과(STT 텍스트 등)를 처리하면 됩니다.
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