// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MumulGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFindSessionCompleted, bool, bWasSuccessful);

UCLASS()
class MUMUL_API UMumulGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	// 생성자
	UMumulGameInstance();

	// UGameInstance 오버라이드
	virtual void Init() override;

	// 세션 생성 시작 함수
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void CreateGameSession(FString SessionName, int32 MaxPlayers, bool bIsLAN, FString TravelURL);

	// 세션 검색 시작 함수
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void FindGameSessions();

	// 세션 참여 함수
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void JoinGameSession(int32 SessionIndex);

	// 레벨 이동 함수 (호스트/클라이언트 모두 사용 가능)
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void TravelToLevel(const FString& LevelName);

	// 세션 생성 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Multiplayer|Delegates")
	FOnSessionCreated OnSessionCreated;

	UFUNCTION()
	void OnFindSessionsComplete(bool bWasSuccessful);

	UPROPERTY(BlueprintAssignable)
	FOnFindSessionCompleted OnFindSessionsCompleteEvent;

	TSharedPtr<FOnlineSessionSearch> GetSessionSearch() { return SessionSearch; }

	UFUNCTION(BlueprintCallable)
	FString GetSteamNickname();
private:
	// Online Session Interface 포인터
	IOnlineSessionPtr SessionInterface;

	// 세션 검색 결과 저장
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// 세션 관련 델리게이트 바인딩
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful); // 세션 재사용을 위해 파괴 후 생성 로직에 필요

	// 실제 세션 생성 함수
	void InternalCreateSession(FName SessionName, int32 MaxPlayers, bool bIsLAN);


	// 생성할 세션의 이름 및 이동할 URL (세션 생성 로직에 필요)
	FName RequestedSessionName;
	FString RequestedTravelURL;
};
