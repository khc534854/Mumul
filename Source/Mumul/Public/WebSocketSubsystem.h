// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "IWebSocket.h"
#include "WebSocketSubsystem.generated.h"

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

	// [핵심] 서버 연결 함수
	UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
	void Connect(const FString& Url);

	// [핵심] 연결 종료 함수
	UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
	void Close();

	// [핵심] 메시지 전송 함수
	UFUNCTION(BlueprintCallable, Category = "Network|WebSocket")
	void SendMessage(const FString& Message);

	// 연결 상태 확인
	UFUNCTION(BlueprintPure, Category = "Network|WebSocket")
	bool IsConnected() const;

public:
	// 외부에서 바인딩할 델리게이트들
	UPROPERTY(BlueprintAssignable)
	FOnWebSocketConnected OnConnected;

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketClosed OnClosed;

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketMessage OnMessageReceived;

	UPROPERTY(BlueprintAssignable)
	FOnWebSocketError OnError;

private:
	// 실제 웹소켓 객체 (포인터로 관리)
	TSharedPtr<IWebSocket> WebSocket;
};
