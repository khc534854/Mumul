#include "WebSocketSubsystem.h"
#include "WebSocketsModule.h" // 모듈 헤더
#include "Async/Async.h"

void UWebSocketSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[WebSocket] Subsystem Initialized"));
}

void UWebSocketSubsystem::Deinitialize()
{
    // 게임 꺼지거나 서브시스템 내려갈 때 연결 정리
    Close();
    Super::Deinitialize();
}

void UWebSocketSubsystem::Connect(const FString& Url)
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

    UE_LOG(LogTemp, Log, TEXT("[WebSocket] Connecting to: %s"), *Url);

    // 1. 소켓 생성
    WebSocket = FWebSocketsModule::Get().CreateWebSocket(Url);

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
        // 2. "event" 필드 확인
        FString EventType = JsonObject->GetStringField(TEXT("event"));

        if (EventType == TEXT("chat_started"))
        {
            FString Msg = JsonObject->GetStringField(TEXT("message"));
            OnAIChatStarted.Broadcast(Msg);
            UE_LOG(LogTemp, Log, TEXT("[WS] Chat Started: %s"), *Msg);
        }
        else if (EventType == TEXT("answer"))
        {
            FString Answer = JsonObject->GetStringField(TEXT("answer"));
            OnAIChatAnswer.Broadcast(Answer);
            UE_LOG(LogTemp, Log, TEXT("[WS] AI Answer: %s"), *Answer);
        }
        else if (EventType == TEXT("chat_ended"))
        {
            FString Msg = JsonObject->GetStringField(TEXT("message"));
            OnAIChatEnded.Broadcast(Msg);
            UE_LOG(LogTemp, Log, TEXT("[WS] Chat Ended: %s"), *Msg);
        }
    }
}
