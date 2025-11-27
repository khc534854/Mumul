// Fill out your copyright notice in the Description page of Project Settings.


#include "khc/System/NetworkTestActor.h"
#include "HttpNetworkSubsystem.h"
#include "WebSocketSubsystem.h" // 필요시 사용
#include "khc/System/NetworkStructs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"
#include "JsonObjectConverter.h" // [필수] JSON 변환용
#include "Library/MumulVoiceFunctionLibrary.h"

ANetworkTestActor::ANetworkTestActor()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ANetworkTestActor::BeginPlay()
{
    Super::BeginPlay();
}

// [헬퍼] 서브시스템을 가져오고 URL 설정을 적용하는 함수
UHttpNetworkSubsystem* ANetworkTestActor::PrepareSubsystem()
{
    UGameInstance* GI = GetGameInstance();
    if (!GI) 
    {
        UE_LOG(LogTemp, Error, TEXT("[Test] GameInstance Not Found! Please PLAY the game first."));
        return nullptr;
    }

    UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>();
    if (!HttpSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("[Test] HttpNetworkSubsystem Not Found!"));
        return nullptr;
    }

    // URL 오버라이드 설정이 켜져있으면 액터의 설정값을 서브시스템에 적용
    if (bOverrideBaseURL)
    {
        // 끝에 슬래시가 있다면 제거 (URL 결합 시 중복 방지)
        if (TargetServerURL.EndsWith("/"))
        {
            TargetServerURL.RemoveAt(TargetServerURL.Len() - 1);
        }
        HttpSystem->BaseURL = TargetServerURL;
        UE_LOG(LogTemp, Log, TEXT("[Test] BaseURL Updated to: %s"), *HttpSystem->BaseURL);
    }

    return HttpSystem;
}

// 1. 로그인 테스트
void ANetworkTestActor::TestSendLogin()
{
    if (UHttpNetworkSubsystem* HttpSystem = PrepareSubsystem())
    {
        FLoginRequest LoginData;
        LoginData.loginId = TestUserID;
        LoginData.password = TestPassword;

        // 디테일 패널에 적은 Endpoint 사용
        HttpSystem->SendJsonRequest(LoginData, Endpoint_Login);
        
        UE_LOG(LogTemp, Log, TEXT("[Test] Login Request Sent: ID=%s"), *TestUserID);
    }
}

// 2. 로그 전송 테스트
void ANetworkTestActor::TestSendLog()
{
    if (UHttpNetworkSubsystem* HttpSystem = PrepareSubsystem())
    {
        FPlayerLogRequest LogData;
        LogData.PlayerID = TestUserID;
        LogData.Timestamp = FDateTime::Now().ToString();

        HttpSystem->SendJsonRequest(LogData, Endpoint_Log);
        
        UE_LOG(LogTemp, Log, TEXT("[Test] Log Request Sent"));
    }
}

// 3. 멀티파트 음성 전송 테스트
void ANetworkTestActor::TestSendMultipartVoice()
{
    if (UHttpNetworkSubsystem* HttpSystem = PrepareSubsystem())
    {
        TArray<uint8> LoadedWavData;
        
        if (UMumulVoiceFunctionLibrary::LoadWavFile(TestFileName, LoadedWavData))
        {
            HttpSystem->SendAudioChunk(LoadedWavData, TestRoomID, TestUserID, 1);
            
            UE_LOG(LogTemp, Log, TEXT("[Test] Real Audio File Sent: %s (Size: %d bytes)"), *TestFileName, LoadedWavData.Num());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[Test] Failed to load file: %s. Please check if the file exists in Saved/RecordedVoice/"), *TestFileName);
        }
    }
}

// void ANetworkTestActor::TestSendLocalFile()
// {
//     if (UHttpNetworkSubsystem* HttpSystem = PrepareSubsystem())
//     {
//         // 2. 파일 로드
//         TArray<uint8> LoadedWavData;
//         if (UMumulVoiceFunctionLibrary::LoadWavFile(TestFileName, LoadedWavData))
//         {
//             // 3. 메타데이터 생성
//             FVoiceMetadata MetaData;
//             MetaData.PlayerName = TestUserID;
//             MetaData.RoomID = TestRoomID;
//             MetaData.TeamID = 1;
//
//             FString MetaJson;
//             FJsonObjectConverter::UStructToJsonObjectString(FVoiceMetadata::StaticStruct(), &MetaData, MetaJson, 0, 0);
//
//             // 4. 기존 멀티파트 전송 함수 재사용!
//             HttpSystem->SendMultipartVoice(LoadedWavData, MetaJson);
//             
//             UE_LOG(LogTemp, Log, TEXT("[Test] Local File Sent: %s"), *TestFileName);
//         }
//         else
//         {
//             UE_LOG(LogTemp, Error, TEXT("[Test] Failed to load local file. Check filename."));
//         }
//     }
// }

TArray<uint8> ANetworkTestActor::GenerateDummyWavData(int32 SizeInBytes)
{
    TArray<uint8> Data;
    Data.Init(0, SizeInBytes);
    
    // 헤더 흉내 (식별용)
    if (SizeInBytes > 4)
    {
        Data[0] = 'R'; Data[1] = 'I'; Data[2] = 'F'; Data[3] = 'F';
    }
    return Data;
}
