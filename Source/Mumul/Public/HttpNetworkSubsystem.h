// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Http.h"
#include "JsonObjectConverter.h"
#include "HttpNetworkSubsystem.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoginResponseReceived, bool, bSuccess, FString, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStartMeetingResponse, bool, bSuccess, FString, MeetingID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJoinMeetingResponse, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndMeetingResponse, bool, bSuccess);

UCLASS()
class MUMUL_API UHttpNetworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 서브시스템 초기화/종료 (필요 시 사용)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	template <typename RequestType>
	void SendJsonRequest(const RequestType& StructData, const FString& UrlEndpoint);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendAudioChunk(const TArray<uint8>& WavData, FString MeetingID, FString UserID, int32 ChunkIndex);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void SendLoginRequest(FString ID, FString PW);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void StartMeetingRequest(FString MeetingTitle, int32 OrganizerID, FString Agenda, FString Desc);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void JoinMeetingRequest(int32 UserID, FString MeetingID);

	UFUNCTION(BlueprintCallable, Category = "Network")
	void EndMeetingRequest(FString MeetingID);
	
	UPROPERTY(BlueprintAssignable)
	FOnLoginResponseReceived OnLoginResponse;
	UPROPERTY(BlueprintAssignable)
	FOnStartMeetingResponse OnStartMeeting;
	UPROPERTY(BlueprintAssignable)
	FOnJoinMeetingResponse OnJoinMeeting;
	UPROPERTY(BlueprintAssignable)
	FOnEndMeetingResponse OnEndMeeting;
	
	
private:
	// 통신이 끝났을 때(응답 왔을 때) 호출될 콜백 함수
	void OnSendVoiceComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnLoginComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnStartMeetingComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnJoinMeetingComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnEndMeetingComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	void AddString(TArray<uint8>& OutPayload, const FString& InString);
public:
	UPROPERTY(EditAnywhere, Category="Network")
	FString BaseURL = TEXT("http://127.0.0.1:8000");

	int64 GetCurrentEpochMs();
};

template <typename RequestType>
void UHttpNetworkSubsystem::SendJsonRequest(const RequestType& StructData, const FString& UrlEndpoint)
{
	FString JsonString;
	bool bSuccess = FJsonObjectConverter::UStructToJsonObjectString(RequestType::StaticStruct(), &StructData, JsonString, 0, 0);

	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("JSON Serialization Failed!"));
		return;
	}

	// 2. HTTP 요청 생성
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    
	// URL 결합 (기본 주소 + 엔드포인트)
	FString FullURL = FString::Printf(TEXT("%s/%s"), *BaseURL, *UrlEndpoint);
    
	Request->SetURL(FullURL);
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(JsonString); // 변환된 JSON 문자열 탑재

	// 3. 콜백 연결 및 전송
	Request->OnProcessRequestComplete().BindUObject(this, &UHttpNetworkSubsystem::OnSendVoiceComplete);

	UE_LOG(LogTemp, Log, TEXT("[HTTP] Sending Request to: %s"), *FullURL);
	Request->ProcessRequest();
}
