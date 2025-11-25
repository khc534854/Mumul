// Fill out your copyright notice in the Description page of Project Settings.


#include "khc/System/NetworkTestActor.h"
#include "HttpNetworkSubsystem.h"
#include "WebSocketSubsystem.h"
#include "khc/System/NetworkStructs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/DateTime.h"

ANetworkTestActor::ANetworkTestActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ANetworkTestActor::BeginPlay()
{
	Super::BeginPlay();
}

void ANetworkTestActor::TestSendLogin()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) 
	{
		UE_LOG(LogTemp, Error, TEXT("[Test] GameInstance Not Found! Play the game first."));
		return;
	}

	if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
	{
		// 로그인 요청 구조체 생성
		FLoginRequest LoginData;
		LoginData.UserID = TestUserID;
		LoginData.Password = TEXT("1234password");

		// 전송
		HttpSystem->SendJsonRequest(LoginData, TEXT("login"));
		UE_LOG(LogTemp, Log, TEXT("[Test] Login Request Sent: %s"), *TestUserID);
	}
}

void ANetworkTestActor::TestSendLog()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
	{
		FPlayerLogRequest LogData;
		LogData.PlayerID = TestUserID;
		LogData.Timestamp = FDateTime::Now().ToString();

		HttpSystem->SendJsonRequest(LogData, TEXT("log-player"));
		UE_LOG(LogTemp, Log, TEXT("[Test] Log Request Sent"));
	}
}

void ANetworkTestActor::TestSendMultipartVoice()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
	{
		// 1. 가짜 WAV 데이터 생성 (10KB)
		TArray<uint8> DummyWav = GenerateDummyWavData(10240);

		// 2. 메타데이터 JSON 문자열 생성
		FString MetaJson = FString::Printf(
			TEXT("{\"player_name\": \"%s\", \"room_id\": \"%s\", \"team_id\": 1}"), 
			*TestUserID, *TestRoomID
		);

		// 3. 전송
		HttpSystem->SendMultipartVoice(DummyWav, MetaJson);
		UE_LOG(LogTemp, Log, TEXT("[Test] Multipart Voice Sent (Dummy Size: 10KB)"));
	}
}

TArray<uint8> ANetworkTestActor::GenerateDummyWavData(int32 SizeInBytes)
{
	TArray<uint8> Data;
	Data.Init(0, SizeInBytes); // 0으로 채운 데이터
    
	// 식별을 위해 앞부분에 간단한 헤더 흉내
	if (SizeInBytes > 4)
	{
		Data[0] = 'D'; Data[1] = 'U'; Data[2] = 'M'; Data[3] = 'M';
	}
	return Data;
}
