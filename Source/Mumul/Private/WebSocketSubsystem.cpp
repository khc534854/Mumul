#include "WebSocketSubsystem.h"
#include "WebSocketsModule.h" // 모듈 헤더

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

    // 2. 이벤트 바인딩
    
    // 연결 성공 시
    WebSocket->OnConnected().AddLambda([this]()
    {
        UE_LOG(LogTemp, Log, TEXT("[WebSocket] Connected!"));
        OnConnected.Broadcast();
    });

    // 연결 실패 시
    WebSocket->OnConnectionError().AddLambda([this](const FString& Error)
    {
        UE_LOG(LogTemp, Error, TEXT("[WebSocket] Connection Error: %s"), *Error);
        OnError.Broadcast(Error);
    });

    // 연결 종료 시
    WebSocket->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
    {
        UE_LOG(LogTemp, Warning, TEXT("[WebSocket] Closed. Code: %d, Reason: %s"), StatusCode, *Reason);
        OnClosed.Broadcast(StatusCode);
    });

    // 메시지 수신 시 (서버가 나한테 뭘 보냈을 때)
    WebSocket->OnMessage().AddLambda([this](const FString& Message)
    {
        UE_LOG(LogTemp, Log, TEXT("[WebSocket] Received: %s"), *Message);
        OnMessageReceived.Broadcast(Message);
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