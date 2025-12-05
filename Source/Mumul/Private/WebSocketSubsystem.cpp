#include "WebSocketSubsystem.h"
#include "MumulGameSettings.h"
#include "WebSocketsModule.h"
#include "Async/Async.h"
#include "Serialization/JsonSerializer.h" // [필수] JSON 처리용

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
    
    // BaseURL 뒤에 슬래시가 있는지 확인 후 결합
    FString FullURL = FString::Printf(TEXT("%s/%s/"), *BaseURL, *EndPoint);

    WebSocket = FWebSocketsModule::Get().CreateWebSocket(FullURL);
    TWeakObjectPtr<UWebSocketSubsystem> WeakThis(this);
    
    UE_LOG(LogTemp, Log, TEXT("[WebSocket] Connecting to: %s"), *FullURL);


    // 1. 연결 성공
    WebSocket->OnConnected().AddLambda([WeakThis]()
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis]()
        {
            // 살아있는지 확인 (IsValid 대신 Get() 유무 확인)
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

    // 3. 연결 종료 (여기가 크래시 지점)
    WebSocket->OnClosed().AddLambda([WeakThis](int32 StatusCode, const FString& Reason, bool bWasClean)
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis, StatusCode, Reason]()
        {
            // 서브시스템이 살아있을 때만 방송
            if (UWebSocketSubsystem* StrongThis = WeakThis.Get())
            {
                // 1000번대(정상)가 아니면 로그
                if (StatusCode != 1000)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[WebSocket] Closed Abnormally. Code: %d, Reason: %s"), StatusCode, *Reason);
                }
                else
                {
                    UE_LOG(LogTemp, Log, TEXT("[WebSocket] Closed Normally."));
                }
                
                StrongThis->OnClosed.Broadcast(StatusCode);
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

    // 1. JSON 파싱 시도
    if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
    {
        FString EventType = JsonObject->GetStringField(TEXT("event"));

        // [이벤트 분기 처리]

        if (EventType == TEXT("chat_started"))
        {
            FString Msg = JsonObject->GetStringField(TEXT("message"));
            UE_LOG(LogTemp, Log, TEXT("[WS] Chat Started: %s"), *Msg);
            OnAIChatStarted.Broadcast(Msg);
        }
        else if (EventType == TEXT("answer"))
        {
            FString AnswerText = JsonObject->GetStringField(TEXT("answer"));
            FString GroupID = JsonObject->GetStringField(TEXT("groupId")); // [추가]

            // groupId가 없으면 빈 문자열로 보냄
            UE_LOG(LogTemp, Log, TEXT("[WS] AI Answer: %s"), *AnswerText);
            OnAIChatAnswer.Broadcast(AnswerText, GroupID);

        }
        else if (EventType == TEXT("chat_ended"))
        {
            FString Msg = JsonObject->GetStringField(TEXT("message"));
            UE_LOG(LogTemp, Log, TEXT("[WS] Chat Ended: %s"), *Msg);
            OnAIChatEnded.Broadcast(Msg);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[WS] Unknown Event Type: %s"), *EventType);
        }
    }
    else
    {
        // 2. JSON이 아닐 경우 (단순 텍스트 메시지 처리)
        if (Message.Contains(TEXT("Welcome client")))
        {
            // 서버의 연결 환영 메시지는 에러가 아님
            UE_LOG(LogTemp, Log, TEXT("[WS] Server Handshake Message: %s"), *Message);
        }
        else
        {
            // 그 외의 파싱 실패는 에러로 출력
            UE_LOG(LogTemp, Error, TEXT("[WS] JSON Parsing Failed. Raw Message: %s"), *Message);
        }
    }
}