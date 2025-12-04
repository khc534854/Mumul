#include "WebSocketSubsystem.h"

#include "MumulGameSettings.h"
#include "WebSocketsModule.h" // 모듈 헤더
#include "Async/Async.h"

class UMumulGameSettings;

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
    // 게임 꺼지거나 서브시스템 내려갈 때 연결 정리
    Close();
    Super::Deinitialize();
}

void UWebSocketSubsystem::Connect(FString EndPoint)
{
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::Get().LoadModule("WebSockets");
    }

    // 이미 연결되어 있다면 정리 후 재연결
    if (WebSocket.IsValid() && WebSocket->IsConnected())
    {
        Close();
    }
    
    FString FullURL = FString::Printf(TEXT("%s/%s/"), *BaseURL, *EndPoint);
    
    UE_LOG(LogTemp, Log, TEXT("[WebSocket] Connecting to: %s"), *FullURL);

    // 1. 소켓 생성
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(FullURL);

    // 2. 이벤트 바인딩 (AsyncTask 적용)

    // [연결 성공]
    WebSocket->OnConnected().AddLambda([this]()
    {
        // 게임 스레드로 작업을 넘김
        AsyncTask(ENamedThreads::GameThread, [this]()
        {
            if (!IsValid(this)) return; // 안전장치
            
            UE_LOG(LogTemp, Log, TEXT("[WebSocket] Connected!"));
            OnConnected.Broadcast();
        });
    });

    // [연결 실패]
    WebSocket->OnConnectionError().AddLambda([this](const FString& Error)
    {
        AsyncTask(ENamedThreads::GameThread, [this, Error]()
        {
            if (!IsValid(this)) return;

            UE_LOG(LogTemp, Error, TEXT("[WebSocket] Connection Error: %s"), *Error);
            OnError.Broadcast(Error);
        });
    });

    // [연결 종료]
    WebSocket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
    {
        AsyncTask(ENamedThreads::GameThread, [this, StatusCode, Reason]()
        {
            if (!IsValid(this)) return;

            UE_LOG(LogTemp, Warning, TEXT("[WebSocket] Closed. Code: %d, Reason: %s"), StatusCode, *Reason);
            OnClosed.Broadcast(StatusCode);
        });
    });

    // [메시지 수신]
    WebSocket->OnMessage().AddLambda([this](const FString& Message)
    {
        AsyncTask(ENamedThreads::GameThread, [this, Message]()
        {
            if (!IsValid(this)) return;

            UE_LOG(LogTemp, Log, TEXT("[WebSocket] Received: %s"), *Message);
            
            // 델리게이트 전파
            OnMessageReceived.Broadcast(Message);
            // 내부 파싱 로직
            HandleWebSocketMessage(Message); 
        });
    });

    // 3. 실제 연결 시작
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
        FString EventType = JsonObject->GetStringField(TEXT("event"));

        // 1. 채팅 시작됨 (chat_started)
        if (EventType == TEXT("chat_started"))
        {
            FString Msg = JsonObject->GetStringField(TEXT("message"));
            // 필요시 sessionId, userId도 파싱 가능
            
            UE_LOG(LogTemp, Log, TEXT("[WS] Chat Started: %s"), *Msg);
            OnAIChatStarted.Broadcast(Msg);
        }
        // 2. 답변 수신 (answer)
        else if (EventType == TEXT("answer"))
        {
            FString AnswerText = JsonObject->GetStringField(TEXT("answer"));
            // 명세서에 추가된 userId 등도 필요하면 파싱
            // FString UserID = JsonObject->GetStringField(TEXT("userId"));

            UE_LOG(LogTemp, Log, TEXT("[WS] AI Answer: %s"), *AnswerText);
            OnAIChatAnswer.Broadcast(AnswerText);
        }
        // 3. 채팅 종료됨 (chat_ended)
        else if (EventType == TEXT("chat_ended"))
        {
            FString Msg = JsonObject->GetStringField(TEXT("message"));
            
            UE_LOG(LogTemp, Log, TEXT("[WS] Chat Ended: %s"), *Msg);
            OnAIChatEnded.Broadcast(Msg);
        }
        // 예외: 에러 메시지나 알 수 없는 이벤트
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[WS] Unknown Event: %s"), *EventType);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[WS] JSON Parsing Failed: %s"), *Message);
    }
}
