// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "JsonObjectConverter.h"
#include "WebSocketSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIChatAnswer, FString, Answer, FString, GroupId); // 답변 왔을 때
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIChatStarted, FString, Message); // 시작됨
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIChatEnded, FString, Message);   // 종료됨

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketClosed, int32, StatusCode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessage, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketError, const FString&, ErrorMsg);

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

	// [추가] 구조체를 JSON으로 변환해서 보내는 템플릿 함수
	template <typename StructType>
	void SendStructMessage(const StructType& InStruct);

	UFUNCTION(BlueprintPure, Category = "Network|WebSocket")
	bool IsConnected() const;

public:
	UPROPERTY(EditAnywhere, Category="Network")
	FString BaseURL = TEXT("ws://127.0.0.1:8000/ws");

public:
	UPROPERTY(BlueprintAssignable)
	FOnWebSocketConnected OnConnected;

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketClosed OnClosed;

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketMessage OnMessageReceived; // 원본 메시지 알림

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketError OnError;

	// [추가] AI 채팅용 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnAIChatAnswer OnAIChatAnswer;

	UPROPERTY(BlueprintAssignable)
	FOnAIChatStarted OnAIChatStarted;
    
	UPROPERTY(BlueprintAssignable)
	FOnAIChatEnded OnAIChatEnded;

private:
	TSharedPtr<IWebSocket> WebSocket;

	// [추가] 수신된 메시지 파싱 함수
	void HandleWebSocketMessage(const FString& Message);
};

// 템플릿 함수 구현
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
