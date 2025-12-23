#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "JsonObjectConverter.h"
#include "Network/NetworkStructs.h" // 위에서 정의한 구조체 포함
#include "WebSocketSubsystem.generated.h"

// 델리게이트 정의
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketClosed, int32, StatusCode);

// System
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemRegistered, int32, UserId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSystemError, FString, ErrorMessage);

// Meeting
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeetingChatStarted, const FMeetingResponsePayload&, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeetingAnswer, const FMeetingResponsePayload&, Answer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeetingChatEnded, const FMeetingResponsePayload&, Info);

// Learning
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLearningChatStarted, const FLearningResponsePayload&, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLearningAnswer, const FLearningResponsePayload&, Answer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLearningChatEnded, const FLearningResponsePayload&, Info);

// Dispatch
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDispatchNotice, const FDispatchNoticePayload&, Notice);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDispatchDM, const FDispatchDMPayload&, DM);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPongReceived);

UCLASS()
class MUMUL_API UWebSocketSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Connection ---
    UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
    void Connect();

    UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
    void Close();

    UFUNCTION(BlueprintPure, Category = "Network|WebSocket")
    bool IsConnected() const;

    // --- A. System ---
    UFUNCTION(BlueprintCallable, Category = "Network|WS|System")
    void RegisterUser(int32 UserId);

    // --- B. Meeting ---
    UFUNCTION(BlueprintCallable, Category = "Network|WS|Meeting")
    void StartMeetingChat(FString GroupId, int32 UserId, FString UserName);

    UFUNCTION(BlueprintCallable, Category = "Network|WS|Meeting")
    void QueryMeetingChat(FString GroupId, int32 UserId, FString UserName, FString Query, FString MeetingId = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Network|WS|Meeting")
    void EndMeetingChat(FString GroupId);

    // --- C. Learning ---
    UFUNCTION(BlueprintCallable, Category = "Network|WS|Learning")
    void StartLearningChat(int32 SessionId, int32 UserId);

    UFUNCTION(BlueprintCallable, Category = "Network|WS|Learning")
    void QueryLearningChat(int32 SessionId, int32 UserId, FString Query, int32 Grade);

    UFUNCTION(BlueprintCallable, Category = "Network|WS|Learning")
    void EndLearningChat(int32 SessionId, int32 UserId);

    // --- D. Dispatch ---
    UFUNCTION(BlueprintCallable, Category = "Network|WS|Dispatch")
    void SendDispatchAck(FString Kind, FString MessageId);

    UFUNCTION(BlueprintCallable, Category = "Network|WS|Dispatch")
    void SendPing();

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Network")
    FString BaseURL = TEXT("ws://127.0.0.1:8000");

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable) FOnWebSocketConnected OnConnected;
    UPROPERTY(BlueprintAssignable) FOnWebSocketClosed OnClosed;
    
    UPROPERTY(BlueprintAssignable) FOnSystemRegistered OnSystemRegistered;
    UPROPERTY(BlueprintAssignable) FOnSystemError OnSystemError;
    
    UPROPERTY(BlueprintAssignable) FOnMeetingChatStarted OnMeetingChatStarted;
    UPROPERTY(BlueprintAssignable) FOnMeetingAnswer OnMeetingAnswer;
    UPROPERTY(BlueprintAssignable) FOnMeetingChatEnded OnMeetingChatEnded;

    UPROPERTY(BlueprintAssignable) FOnLearningChatStarted OnLearningChatStarted;
    UPROPERTY(BlueprintAssignable) FOnLearningAnswer OnLearningAnswer;
    UPROPERTY(BlueprintAssignable) FOnLearningChatEnded OnLearningChatEnded;

    UPROPERTY(BlueprintAssignable) FOnDispatchNotice OnDispatchNotice;
    UPROPERTY(BlueprintAssignable) FOnDispatchDM OnDispatchDM;
    UPROPERTY(BlueprintAssignable) FOnPongReceived OnPongReceived;

private:
    TSharedPtr<IWebSocket> WebSocket;
    
    // 재연결 관리
    FTimerHandle ReconnectTimerHandle;
    bool bShouldBeConnected = false; 

    void StartReconnectTimer();
    void StopReconnectTimer();
    void TryReconnect();

    // 메시지 라우터
    void HandleWebSocketMessage(const FString& Message);
    
    // 도메인별 처리
    void HandleSystemMessage(const FString& Event, TSharedPtr<FJsonObject> PayloadObj);
    void HandleMeetingMessage(const FString& Event, TSharedPtr<FJsonObject> PayloadObj);
    void HandleLearningMessage(const FString& Event, TSharedPtr<FJsonObject> PayloadObj);
    void HandleDispatchMessage(const FString& Event, TSharedPtr<FJsonObject> PayloadObj);

    // 공통 전송 함수 (Envelope 포장)
    template <typename T>
    void SendEnvelope(const FString& Domain, const FString& Event, const T& PayloadStruct);
    
    // Payload 없는 전송
    void SendEnvelope(const FString& Domain, const FString& Event);
};

// Template Implementation
template <typename T>
void UWebSocketSubsystem::SendEnvelope(const FString& Domain, const FString& Event, const T& PayloadStruct)
{
    if (!IsConnected()) return;

    TSharedPtr<FJsonObject> RootObj = MakeShared<FJsonObject>();
    RootObj->SetStringField(TEXT("domain"), Domain);
    RootObj->SetStringField(TEXT("event"), Event);

    TSharedPtr<FJsonObject> PayloadObj = FJsonObjectConverter::UStructToJsonObject(PayloadStruct);
    RootObj->SetObjectField(TEXT("payload"), PayloadObj);

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);

    WebSocket->Send(JsonString);
}