#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "JsonObjectConverter.h"
#include "WebSocketSubsystem.generated.h"

// [신규] 챗봇 타입 열거형
UENUM(BlueprintType)
enum class EWebSocketChatbotType : uint8
{
    None,
    Learning,   // 학습 챗봇 (1:1)
    Meeting,    // 회의 도우미 (그룹)
    Notice,     // 공지
};

// [기존] 공통 연결 관련 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketClosed, int32, StatusCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessage, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketError, const FString&, ErrorMsg);

// [신규] 학습 챗봇용 델리게이트 (Learning)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLearningChatStarted, FString, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLearningChatAnswer, FString, Answer); // 학습은 1:1이라 ID 불필요
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLearningChatEnded, FString, Message);

// [신규] 회의 챗봇용 델리게이트 (Meeting)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnMeetingChatStarted, FString, Message, FString, GroupId, FString, UserName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeetingChatAnswer, FString, Answer, FString, GroupId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeetingChatEnded, FString, Message, FString, GroupId);

// 공지 델리게이트 (Notice)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoticeReceived, const FString&, NoticeMessage);


UCLASS()
class MUMUL_API UWebSocketSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
    void Connect(FString EndPoint);

    UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
    void Close();

    UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
    void SendMessage(const FString& Message);

    template <typename StructType>
    void SendStructMessage(const StructType& InStruct);

    UFUNCTION(BlueprintPure, Category = "Network|WebSocket")
    bool IsConnected() const;

public:
    UPROPERTY(EditAnywhere, Category="Network")
    FString BaseURL = TEXT("ws://127.0.0.1:8000/ws");

    // [신규] 현재 연결된 챗봇 타입
    UPROPERTY(BlueprintReadOnly, Category="Network|WebSocket")
    EWebSocketChatbotType CurrentChatbotType = EWebSocketChatbotType::None;

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable)
    FOnWebSocketConnected OnConnected;
    UPROPERTY(BlueprintAssignable)
    FOnWebSocketClosed OnClosed;
    UPROPERTY(BlueprintAssignable)
    FOnWebSocketMessage OnMessageReceived;
    UPROPERTY(BlueprintAssignable)
    FOnWebSocketError OnError;

    // 학습 챗봇 이벤트
    UPROPERTY(BlueprintAssignable)
    FOnLearningChatStarted OnLearningChatStarted;
    UPROPERTY(BlueprintAssignable)
    FOnLearningChatAnswer OnLearningChatAnswer;
    UPROPERTY(BlueprintAssignable)
    FOnLearningChatEnded OnLearningChatEnded;

    // 회의 챗봇 이벤트
    UPROPERTY(BlueprintAssignable)
    FOnMeetingChatStarted OnMeetingChatStarted;
    UPROPERTY(BlueprintAssignable)
    FOnMeetingChatAnswer OnMeetingChatAnswer;
    UPROPERTY(BlueprintAssignable)
    FOnMeetingChatEnded OnMeetingChatEnded;

    // 공지 이벤트
    UPROPERTY(BlueprintAssignable)
    FOnNoticeReceived OnNoticeReceived;
    
private:
    TSharedPtr<IWebSocket> WebSocket;

    void HandleWebSocketMessage(const FString& Message);
    
    // 내부 분리 처리 함수
    void HandleLearningMessage(TSharedPtr<FJsonObject> JsonObject);
    void HandleMeetingMessage(TSharedPtr<FJsonObject> JsonObject);
    void HandleNoticeMessage(TSharedPtr<FJsonObject> JsonObject);
};

template <typename StructType>
void UWebSocketSubsystem::SendStructMessage(const StructType& InStruct)
{
    FString JsonString;
    if (FJsonObjectConverter::UStructToJsonObjectString(StructType::StaticStruct(), &InStruct, JsonString, 0, 0))
    {
        SendMessage(JsonString);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[WebSocket] Failed to serialize struct to JSON"));
    }
}