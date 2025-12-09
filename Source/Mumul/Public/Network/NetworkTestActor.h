// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Network/WebSocketSubsystem.h"
#include "GameFramework/Actor.h"
#include "NetworkTestActor.generated.h"

UCLASS()
class MUMUL_API ANetworkTestActor : public AActor
{
    GENERATED_BODY()
    
public:    
    ANetworkTestActor();

protected:
    virtual void BeginPlay() override;

public:
    // ==============================================================================
    // [설정 1] 서버 설정 (여기서 URL을 바꾸면 서브시스템에 적용됨)
    // ==============================================================================
    UPROPERTY(EditAnywhere, Category = "HTTP | Server Config")
    bool bOverrideBaseURL = true; // 체크하면 아래 URL을 사용, 끄면 서브시스템 기본값 사용

    UPROPERTY(EditAnywhere, Category = "HTTP | Server Config", meta = (EditCondition = "bOverrideBaseURL"))
    FString TargetServerURL = TEXT("http://127.0.0.1:8000");

    UPROPERTY(EditAnywhere, Category = "HTTP | Server Config")
    FString Endpoint_Login = TEXT("login");

    UPROPERTY(EditAnywhere, Category = "HTTP | Server Config")
    FString Endpoint_Log = TEXT("log-player");

    UPROPERTY(EditAnywhere, Category = "HTTP | Server Config")
    FString Endpoint_Voice = TEXT("upload-multipart");

    // ==============================================================================
    // [설정 2] 테스트 데이터 (보낼 내용)
    // ==============================================================================
    UPROPERTY(EditAnywhere, Category = "HTTP | Test Data")
    FString TestUserID = TEXT("User_01");
    
    UPROPERTY(EditAnywhere, Category = "HTTP | Test Data")
    int32 TestUserIDX = 1;
    

    UPROPERTY(EditAnywhere, Category = "HTTP | Test Data")
    FString TestPassword = TEXT("1234password");

    UPROPERTY(EditAnywhere, Category = "HTTP | Test Data")
    FString TestRoomID = TEXT("Room_Alpha");

    UPROPERTY(EditAnywhere, Category = "HTTP | Test Data", meta = (ClampMin = "1024"))
    int32 DummyFileSize = 10240; // 전송할 가짜 음성 파일 크기 (Byte)
    
    UPROPERTY(EditAnywhere, Category = "HTTP | Test Data")
    FString TestFileName = TEXT("TestRecord.wav");

    // ==============================================================================
    // [설정 3] WebSocket 테스트 데이터
    // ==============================================================================
    
    UPROPERTY(EditAnywhere, Category = "WebSocket | Config")
    FString WebSocketURL = TEXT("ws://127.0.0.1:8000/ws"); // 서버 주소

    UPROPERTY(EditAnywhere, Category = "WebSocket | Data")
    FString WS_SessionID = TEXT("10");

    UPROPERTY(EditAnywhere, Category = "WebSocket | Data")
    FString WS_UserID = TEXT("user123");

    UPROPERTY(EditAnywhere, Category = "WebSocket | Data")
    FString WS_QueryText = TEXT("파이썬 함수를 쉽게 설명해줘");
    
    // ==============================================================================
    // [기능] 테스트 실행 버튼
    // ==============================================================================
    
    // 1. 로그인 테스트
    UFUNCTION(CallInEditor, Category = "HTTP | Actions")
    void TestSendLogin();

    // 2. 로그 전송 테스트
    UFUNCTION(CallInEditor, Category = "HTTP | Actions")
    void TestSendLog();

    // 3. 멀티파트 음성 전송 테스트
    UFUNCTION(CallInEditor, Category = "HTTP | Actions")
    void TestSendMultipartVoice();
    UWebSocketSubsystem* GetWSSubsystem();

    // [추가] 로컬 파일 전송 버튼
    // UFUNCTION(CallInEditor, Category = "HTTP | Actions")
    // void TestSendLocalFile();

    // ==============================================================================
    // [기능] WebSocket 테스트 버튼
    // ==============================================================================
    
    UFUNCTION(CallInEditor, Category = "WebSocket | Actions")
    void TestWS_Connect();

    UFUNCTION(CallInEditor, Category = "WebSocket | Actions")
    void TestWS_StartChat();

    UFUNCTION(CallInEditor, Category = "WebSocket | Actions")
    void TestWS_SendQuery();

    UFUNCTION(CallInEditor, Category = "WebSocket | Actions")
    void TestWS_EndChat();

    UFUNCTION(CallInEditor, Category = "WebSocket | Actions")
    void TestWS_Close();

private:
    // 헬퍼 함수: 테스트 시작 전 URL 설정 등을 처리
    class UHttpNetworkSubsystem* PrepareSubsystem();

    // 더미 데이터 생성
    TArray<uint8> GenerateDummyWavData(int32 SizeInBytes);
};