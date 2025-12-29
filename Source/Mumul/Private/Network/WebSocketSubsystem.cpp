#include "Network/WebSocketSubsystem.h"
#include "Base/MumulGameSettings.h"
#include "WebSocketsModule.h"
#include "TimerManager.h"
#include "Async/Async.h"

void UWebSocketSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    const UMumulGameSettings* Settings = GetDefault<UMumulGameSettings>();
    if (Settings) BaseURL = Settings->WebSocketURL;
}

void UWebSocketSubsystem::Deinitialize()
{
    Close();
    Super::Deinitialize();
}

void UWebSocketSubsystem::Connect()
{
    if (IsConnected()) return;
    bShouldBeConnected = true;

    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::Get().LoadModule("WebSockets");
    }

    // 명세서 Endpoint: ws://{host}/ws
    FString FullURL = FString::Printf(TEXT("%s/ws"), *BaseURL);

    WebSocket = FWebSocketsModule::Get().CreateWebSocket(FullURL);
    TWeakObjectPtr<UWebSocketSubsystem> WeakThis(this);

    UE_LOG(LogTemp, Log, TEXT("[WS] Connecting to: %s"), *FullURL);

    WebSocket->OnConnected().AddLambda([WeakThis]()
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis]()
        {
            if (UWebSocketSubsystem* StrongThis = WeakThis.Get())
            {
                UE_LOG(LogTemp, Log, TEXT("[WS] Connected!"));
                
                // [중요] 연결되자마자 재연결 타이머 끄고, 하트비트 시작
                StrongThis->StopReconnectTimer();
                StrongThis->StartHeartbeat(); 

                StrongThis->OnConnected.Broadcast();
            }
        });
    });

    WebSocket->OnClosed().AddLambda([WeakThis](int32 StatusCode, const FString& Reason, bool bWasClean)
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis, StatusCode]()
        {
            if (UWebSocketSubsystem* StrongThis = WeakThis.Get())
            {
                UE_LOG(LogTemp, Warning, TEXT("[WS] Closed. Code: %d"), StatusCode);
                
                // [중요] 연결 끊기면 하트비트 끄고, 재연결 시도 여부 결정
                StrongThis->StopHeartbeat();
                StrongThis->OnClosed.Broadcast(StatusCode);

                // 우리가 끄라고 명령(Close())한 게 아닌데 끊겼다면 재연결 시도
                if (StrongThis->bShouldBeConnected)
                {
                    StrongThis->StartReconnectTimer();
                }
            }
        });
    });

    WebSocket->OnMessage().AddLambda([WeakThis](const FString& Message)
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis, Message]()
        {
            if (UWebSocketSubsystem* StrongThis = WeakThis.Get())
            {
                StrongThis->HandleWebSocketMessage(Message);
            }
        });
    });

    WebSocket->OnConnectionError().AddLambda([WeakThis](const FString& Error)
    {
        AsyncTask(ENamedThreads::GameThread, [WeakThis, Error]()
        {
            if (UWebSocketSubsystem* StrongThis = WeakThis.Get())
            {
                UE_LOG(LogTemp, Error, TEXT("[WS] Connection Error: %s"), *Error);
                if (StrongThis->bShouldBeConnected) StrongThis->StartReconnectTimer();
            }
        });
    });

    WebSocket->Connect();
}

void UWebSocketSubsystem::Close()
{
    UE_LOG(LogTemp, Error, TEXT("[WS] Connection Close"));
    bShouldBeConnected = false;
    
    StopReconnectTimer();
    StopHeartbeat();

    if (IsConnected())
    {
        WebSocket->Close();
    }
}

bool UWebSocketSubsystem::IsConnected() const
{
    return WebSocket.IsValid() && WebSocket->IsConnected();
}

// ----------------------------------------------------------------------------
// Message Routing Logic
// ----------------------------------------------------------------------------
void UWebSocketSubsystem::HandleWebSocketMessage(const FString& Message)
{
    TSharedPtr<FJsonObject> RootObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[WS] Invalid JSON"));
        return;
    }

    FString Domain = RootObj->GetStringField(TEXT("domain"));
    FString Event = RootObj->GetStringField(TEXT("event"));
    TSharedPtr<FJsonObject> Payload = RootObj->GetObjectField(TEXT("payload"));

    if (!Payload.IsValid()) Payload = MakeShared<FJsonObject>();

    if (Domain == TEXT("system")) HandleSystemMessage(Event, Payload);
    else if (Domain == TEXT("meeting")) HandleMeetingMessage(Event, Payload);
    else if (Domain == TEXT("learning")) HandleLearningMessage(Event, Payload);
    else if (Domain == TEXT("dispatch")) HandleDispatchMessage(Event, Payload);
    else UE_LOG(LogTemp, Warning, TEXT("[WS] Unknown Domain: %s"), *Domain);
}

// ----------------------------------------------------------------------------
// Domain Handlers
// ----------------------------------------------------------------------------
void UWebSocketSubsystem::HandleSystemMessage(const FString& Event, TSharedPtr<FJsonObject> PayloadObj)
{
    if (Event == TEXT("registered"))
    {
        // 성공 응답: {"userId": 123}
        FSystemRegisterPayload Data;
        if (FJsonObjectConverter::JsonObjectToUStruct(PayloadObj.ToSharedRef(), &Data))
        {
            UE_LOG(LogTemp, Log, TEXT("[WS] System Registered Success: UserID %d"), Data.userId);
            OnSystemRegistered.Broadcast(Data.userId);
        }
    }
    else if (Event == TEXT("error"))
    {
        // 실패 응답: {"message": "userId required"}
        FString ErrorMsg = PayloadObj->GetStringField(TEXT("message"));
        UE_LOG(LogTemp, Error, TEXT("[WS] System Error: %s"), *ErrorMsg);
        OnSystemError.Broadcast(ErrorMsg);
    }
}

void UWebSocketSubsystem::HandleMeetingMessage(const FString& Event, TSharedPtr<FJsonObject> PayloadObj)
{
    FMeetingResponsePayload Data;
    FJsonObjectConverter::JsonObjectToUStruct(PayloadObj.ToSharedRef(), &Data);

    if (Event == TEXT("chat_started"))
    {
        UE_LOG(LogTemp, Log, TEXT("[WS Recv] Meeting Started (Group: %s): %s"), *Data.groupId, *Data.message); // 로그
        OnMeetingChatStarted.Broadcast(Data);
    }
    else if (Event == TEXT("answer"))
    {
        UE_LOG(LogTemp, Log, TEXT("[WS Recv] Meeting Answer (Group: %s): %s"), *Data.groupId, *Data.answer); // 로그
        OnMeetingAnswer.Broadcast(Data);
    }
    else if (Event == TEXT("chat_ended"))
    {
        UE_LOG(LogTemp, Log, TEXT("[WS Recv] Meeting Ended (Group: %s): %s"), *Data.groupId, *Data.message); // 로그
        OnMeetingChatEnded.Broadcast(Data);
    }
}

void UWebSocketSubsystem::HandleLearningMessage(const FString& Event, TSharedPtr<FJsonObject> PayloadObj)
{
    FLearningResponsePayload Data;
    FJsonObjectConverter::JsonObjectToUStruct(PayloadObj.ToSharedRef(), &Data);

    if (Event == TEXT("chat_started"))
    {
        UE_LOG(LogTemp, Log, TEXT("[WS Recv] Learning Started: %s"), *Data.message); // 로그
        OnLearningChatStarted.Broadcast(Data);
    }
    else if (Event == TEXT("answer"))
    {
        UE_LOG(LogTemp, Log, TEXT("[WS Recv] Learning Answer: %s"), *Data.answer); // 로그
        OnLearningAnswer.Broadcast(Data);
    }
    else if (Event == TEXT("chat_ended"))
    {
        UE_LOG(LogTemp, Log, TEXT("[WS Recv] Learning Ended: %s"), *Data.message); // 로그
        OnLearningChatEnded.Broadcast(Data);
    }
}

void UWebSocketSubsystem::HandleDispatchMessage(const FString& Event, TSharedPtr<FJsonObject> PayloadObj)
{
    if (Event == TEXT("notice"))
    {
        FDispatchPayloadBase Notice;
        FJsonObjectConverter::JsonObjectToUStruct(PayloadObj.ToSharedRef(), &Notice);
        
        UE_LOG(LogTemp, Log, TEXT("[WS Recv] Notice: [%d] %s"), Notice.MessageId, *Notice.Title); // 로그
        OnDispatchNotice.Broadcast(Notice);
    }
    else if (Event == TEXT("dm"))
    {
        FDispatchPayloadBase DM;
        FJsonObjectConverter::JsonObjectToUStruct(PayloadObj.ToSharedRef(), &DM);
        
        UE_LOG(LogTemp, Log, TEXT("[WS Recv] DM: %s"), *DM.Text); // 로그
        OnDispatchDM.Broadcast(DM);
    }
    else if (Event == TEXT("pong"))
    {
        UE_LOG(LogTemp, Verbose, TEXT("[WS Recv] Pong Received")); // 로그 (Verbose 추천)
        OnPongReceived.Broadcast();
    }
    else if (Event == TEXT("ack_ok"))
    {
        FDispatchAckPongPayload AckPong;
        FJsonObjectConverter::JsonObjectToUStruct(PayloadObj.ToSharedRef(), &AckPong);
        OnAckPongReceived.Broadcast(AckPong);
        UE_LOG(LogTemp, Verbose, TEXT("[WS Recv] ACK OK")); // 로그
    }
}

// ----------------------------------------------------------------------------
// Senders
// ----------------------------------------------------------------------------
void UWebSocketSubsystem::RegisterUser(int32 UserId)
{
    FSystemRegisterPayload Payload;
    Payload.userId = UserId;
    SendEnvelope(TEXT("system"), TEXT("register"), Payload);
    UE_LOG(LogTemp, Log, TEXT("[WS] Sending Register Request for UserID: %d"), UserId);
}

void UWebSocketSubsystem::StartMeetingChat(FString GroupId, int32 UserId, FString UserName)
{
    FMeetingStartPayload Payload;
    Payload.groupId = GroupId;
    Payload.userId = UserId;
    Payload.userName = UserName;
    SendEnvelope(TEXT("meeting"), TEXT("start_chat"), Payload);
}

void UWebSocketSubsystem::QueryMeetingChat(FString GroupId, int32 UserId, FString UserName, FString Query, FString MeetingId)
{
    FMeetingQueryPayload Payload;
    Payload.groupId = GroupId;
    Payload.userId = UserId;
    Payload.userName = UserName;
    Payload.query = Query;
    Payload.meeting_id = MeetingId;
    SendEnvelope(TEXT("meeting"), TEXT("query"), Payload);
}

void UWebSocketSubsystem::EndMeetingChat(FString GroupId)
{
    FMeetingEndPayload Payload;
    Payload.groupId = GroupId;
    SendEnvelope(TEXT("meeting"), TEXT("end_chat"), Payload);
}

void UWebSocketSubsystem::StartLearningChat(int32 SessionId, int32 UserId)
{
    FLearningStartPayload Payload;
    Payload.sessionId = SessionId;
    Payload.userId = UserId;
    SendEnvelope(TEXT("learning"), TEXT("start_chat"), Payload);
}

void UWebSocketSubsystem::QueryLearningChat(int32 SessionId, int32 UserId, FString Query, FString Grade)
{
    FLearningQueryPayload Payload;
    Payload.sessionId = SessionId;
    Payload.userId = UserId;
    Payload.query = Query;
    Payload.grade = Grade;
    SendEnvelope(TEXT("learning"), TEXT("query"), Payload);
}

void UWebSocketSubsystem::EndLearningChat(int32 SessionId, int32 UserId)
{
    // EndChat Payload는 Start와 동일 (sessionId, userId)
    FLearningStartPayload Payload; 
    Payload.sessionId = SessionId;
    Payload.userId = UserId;
    SendEnvelope(TEXT("learning"), TEXT("end_chat"), Payload);
}

void UWebSocketSubsystem::SendDispatchAck(FString Kind, int32 MessageId, int32 UserId)
{
    FDispatchAckPayload Payload;
    Payload.kind = Kind;
    Payload.messageId = MessageId;
    Payload.userId = UserId;
    Payload.receivedAt = FDateTime::Now().ToIso8601();
    SendEnvelope(TEXT("dispatch"), TEXT("ack"), Payload);
}

void UWebSocketSubsystem::SendPing()
{
    SendEnvelope(TEXT("dispatch"), TEXT("ping"));
}

void UWebSocketSubsystem::SendEnvelope(const FString& Domain, const FString& Event)
{
    if (!IsConnected()) return;

    TSharedPtr<FJsonObject> RootObj = MakeShared<FJsonObject>();
    RootObj->SetStringField(TEXT("domain"), Domain);
    RootObj->SetStringField(TEXT("event"), Event);
    RootObj->SetObjectField(TEXT("payload"), MakeShared<FJsonObject>());

    FString JsonString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonString);
    FJsonSerializer::Serialize(RootObj.ToSharedRef(), Writer);

    WebSocket->Send(JsonString);
}

// ----------------------------------------------------------------------------
// Reconnection
// ----------------------------------------------------------------------------
void UWebSocketSubsystem::StartReconnectTimer()
{
    if (GetWorld() && !GetWorld()->GetTimerManager().IsTimerActive(ReconnectTimerHandle))
    {
        // 5초 뒤에 재연결 시도
        GetWorld()->GetTimerManager().SetTimer(
            ReconnectTimerHandle, 
            this, 
            &UWebSocketSubsystem::TryReconnect, 
            5.0f, 
            true
        );
        UE_LOG(LogTemp, Warning, TEXT("[WS] Auto-Reconnect Timer Started..."));
    }
}

void UWebSocketSubsystem::StopReconnectTimer()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ReconnectTimerHandle);
    }
}

void UWebSocketSubsystem::TryReconnect()
{
    if (IsConnected())
    {
        StopReconnectTimer();
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[WS] Trying to Reconnect..."));
    Connect();
}

void UWebSocketSubsystem::StartHeartbeat()
{
    // 이미 돌고 있으면 리셋
    StopHeartbeat();

    // 30초마다 Ping 전송 (서버 타임아웃 설정보다 조금 짧게 잡아야 함)
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            HeartbeatTimerHandle, 
            this, 
            &UWebSocketSubsystem::SendHeartbeatPing, 
            30.0f, 
            true // 반복
        );
        UE_LOG(LogTemp, Verbose, TEXT("[WS] Heartbeat Started"));
    }
}

void UWebSocketSubsystem::StopHeartbeat()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(HeartbeatTimerHandle);
    }
}

void UWebSocketSubsystem::SendHeartbeatPing()
{
    if (IsConnected())
    {
        // 명세서에 정의된 Ping 전송
        SendPing(); 
        // 로그가 너무 많이 쌓이면 Verbose로 변경
        // UE_LOG(LogTemp, Verbose, TEXT("[WS] Ping Sent")); 
    }
    else
    {
        // 연결된 줄 알았는데 끊겨있다면? -> 즉시 재연결 절차로 넘어감
        StopHeartbeat();
        StartReconnectTimer();
    }
}