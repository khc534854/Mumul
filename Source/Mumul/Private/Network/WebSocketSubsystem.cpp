#include "Network/WebSocketSubsystem.h"
#include "Base/MumulGameSettings.h"
#include "WebSocketsModule.h"
#include "Async/Async.h"
#include "Serialization/JsonSerializer.h"

void UWebSocketSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[WebSocket] Subsystem Initialized"));

    const UMumulGameSettings* Settings = GetDefault<UMumulGameSettings>();
    if (Settings)
    {
        BaseURL = Settings->WebSocketURL;
    }
}

void UWebSocketSubsystem::Deinitialize()
{
    Close();
    Super::Deinitialize();
}

void UWebSocketSubsystem::Connect(FString EndPoint)
{
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::Get().LoadModule("WebSockets");
    }

    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        Close();
    }
    
    // [신규] 엔드포인트에 따라 챗봇 타입 설정
    if (EndPoint.Contains(TEXT("learning_chatbot")))
    {
        CurrentChatbotType = EWebSocketChatbotType::Learning;
    }
    else if (EndPoint.Contains(TEXT("meeting_chatbot")))
    {
        CurrentChatbotType = EWebSocketChatbotType::Meeting;
    }
    else
    {
        CurrentChatbotType = EWebSocketChatbotType::None;
        UE_LOG(LogTemp, Warning, TEXT("[WebSocket] Unknown Endpoint Type: %s"), *EndPoint);
    }

    // BaseURL 뒤에 슬래시 처리
    FString FullURL = FString::Printf(TEXT("%s/%s/"), *BaseURL, *EndPoint);
    // 만약 BaseURL이 슬래시로 안 끝나고 EndPoint도 슬래시로 시작 안하면 중간에 / 추가 필요
    // 여기서는 간단히 포맷팅
    
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(FullURL);
    TWeakObjectPtr<UWebSocketSubsystem> WeakThis(this);
    
    UE_LOG(LogTemp, Log, TEXT("[WebSocket] Connecting to: %s (Type: %d)"), *FullURL, (int32)CurrentChatbotType);

    // 1. 연결 성공
    WebSocket->OnConnected().AddLambda([WeakThis]()
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis]()
        {
            if (UWebSocketSubsystem* StrongThis = WeakThis.Get())
            {
                UE_LOG(LogTemp, Log, TEXT("[WebSocket] Connected!"));
                StrongThis->OnConnected.Broadcast();
            }
        });
    });

    // 2. 연결 실패
    WebSocket->OnConnectionError().AddLambda([WeakThis](const FString& Error)
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis, Error]()
        {
            if (UWebSocketSubsystem* StrongThis = WeakThis.Get())
            {
                UE_LOG(LogTemp, Error, TEXT("[WebSocket] Connection Error: %s"), *Error);
                StrongThis->OnError.Broadcast(Error);
            }
        });
    });

    // 3. 연결 종료
    WebSocket->OnClosed().AddLambda([WeakThis](int32 StatusCode, const FString& Reason, bool bWasClean)
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis, StatusCode, Reason]()
        {
            if (UWebSocketSubsystem* StrongThis = WeakThis.Get())
            {
                if (StatusCode != 1000)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[WebSocket] Closed Abnormally. Code: %d, Reason: %s"), StatusCode, *Reason);
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("[WebSocket] Closed Normally."));
                }
                
                StrongThis->OnClosed.Broadcast(StatusCode);
                StrongThis->CurrentChatbotType = EWebSocketChatbotType::None; // 타입 초기화
            }
        });
    });

    // 4. 메시지 수신
    WebSocket->OnMessage().AddLambda([WeakThis](const FString& Message)
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis, Message]()
        {
            if (UWebSocketSubsystem* StrongThis = WeakThis.Get())
            {
                UE_LOG(LogTemp, Log, TEXT("[WebSocket] Received: %s"), *Message);
                StrongThis->OnMessageReceived.Broadcast(Message);
                StrongThis->HandleWebSocketMessage(Message); 
            }
        });
    });

    WebSocket->Connect();
}

void UWebSocketSubsystem::Close()
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        WebSocket->Close();
    }
}

void UWebSocketSubsystem::SendMessage(const FString& Message)
{
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        WebSocket->Send(Message);
        UE_LOG(LogTemp, Log, TEXT("[WebSocket] Sent: %s"), *Message);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[WebSocket] Cannot send message. Not connected."));
    }
}

bool UWebSocketSubsystem::IsConnected() const
{
    return WebSocket.IsValid() && WebSocket->IsConnected();
}

void UWebSocketSubsystem::HandleWebSocketMessage(const FString& Message)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        // [핵심] 현재 설정된 타입에 따라 다른 처리 함수 호출
        switch (CurrentChatbotType)
        {
        case EWebSocketChatbotType::Learning:
            HandleLearningMessage(JsonObject);
            break;
        case EWebSocketChatbotType::Meeting:
            HandleMeetingMessage(JsonObject);
            break;
        default:
            UE_LOG(LogTemp, Warning, TEXT("[WS] Message received but ChatbotType is None/Unknown."));
            break;
        }
    }
    else
    {
        // JSON 파싱 실패 혹은 단순 텍스트 메시지
        if (Message.Contains(TEXT("Welcome client")))
        {
            UE_LOG(LogTemp, Log, TEXT("[WS] Handshake: %s"), *Message);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[WS] JSON Parsing Failed."));
        }
    }
}

// [1] 학습 챗봇 메시지 처리 (1:1)
void UWebSocketSubsystem::HandleLearningMessage(TSharedPtr<FJsonObject> JsonObject)
{
    FString EventType = JsonObject->GetStringField(TEXT("event"));

    if (EventType == TEXT("chat_started"))
    {
        FString Msg = JsonObject->GetStringField(TEXT("message"));
        OnLearningChatStarted.Broadcast(Msg);
    }
    else if (EventType == TEXT("answer"))
    {
        FString AnswerText = JsonObject->GetStringField(TEXT("answer"));
        OnLearningChatAnswer.Broadcast(AnswerText);
    }
    else if (EventType == TEXT("chat_ended"))
    {
        FString Msg = JsonObject->GetStringField(TEXT("message"));
        OnLearningChatEnded.Broadcast(Msg);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[WS-Learning] Unknown Event: %s"), *EventType);
    }
}

// [2] 회의 챗봇 메시지 처리 (그룹)
void UWebSocketSubsystem::HandleMeetingMessage(TSharedPtr<FJsonObject> JsonObject)
{
    FString EventType = JsonObject->GetStringField(TEXT("event"));
    FString GroupId = JsonObject->GetStringField(TEXT("groupId"));

    if (EventType == TEXT("chat_started"))
    {
        FString Msg = JsonObject->GetStringField(TEXT("message"));
        FString UserName = JsonObject->GetStringField(TEXT("userName"));
        OnMeetingChatStarted.Broadcast(Msg, GroupId, UserName);
    }
    else if (EventType == TEXT("answer"))
    {
        FString AnswerText = JsonObject->GetStringField(TEXT("answer"));
        OnMeetingChatAnswer.Broadcast(AnswerText, GroupId);
    }
    else if (EventType == TEXT("chat_ended"))
    {
        FString Msg = JsonObject->GetStringField(TEXT("message"));
        OnMeetingChatEnded.Broadcast(Msg, GroupId);
    }
    else if (EventType == TEXT("error"))
    {
         FString ErrorMsg = JsonObject->GetStringField(TEXT("message"));
         UE_LOG(LogTemp, Error, TEXT("[WS-Meeting] Error: %s"), *ErrorMsg);
         OnError.Broadcast(ErrorMsg);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[WS-Meeting] Unknown Event: %s"), *EventType);
    }
}