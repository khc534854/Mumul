// Fill out your copyright notice in the Description page of Project Settings.


#include "Mumul/Public/MumulGameInstance.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"


UMumulGameInstance::UMumulGameInstance()
{
}

void UMumulGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		// 2. 세션 인터페이스 가져오기
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			// 3. 세션 델리게이트 바인딩
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UMumulGameInstance::OnCreateSessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UMumulGameInstance::OnFindSessionsComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UMumulGameInstance::OnJoinSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UMumulGameInstance::OnDestroySessionComplete);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Session Interface is NOT Valid!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Online Subsystem is NOT Available!"));
	}
}

void UMumulGameInstance::CreateGameSession(FString SessionName, int32 MaxPlayers, bool bIsLAN, FString TravelURL)
{
	if (!SessionInterface.IsValid())
	{
		OnSessionCreated.Broadcast(false);
		return;
	}

	RequestedSessionName = FName(*SessionName);
	RequestedTravelURL = TravelURL;
	
	// 세션이 이미 존재하면 파괴 후 생성 (재사용 목적)
	if (SessionInterface->GetNamedSession(RequestedSessionName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Existing session found. Destroying session: %s"), *RequestedSessionName.ToString());
		SessionInterface->DestroySession(RequestedSessionName);
		// OnDestroySessionComplete에서 InternalCreateSession이 호출됨
	}
	else
	{
		// 세션이 없으면 바로 생성
		InternalCreateSession(RequestedSessionName, MaxPlayers, bIsLAN, RequestedTravelURL);
	}
}

void UMumulGameInstance::FindGameSessions()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("Session Interface is NOT Valid for FindSessions!"));
		return;
	}

	// 1. 검색 객체 생성 및 설정
	SessionSearch = MakeShared<FOnlineSessionSearch>();
	if (SessionSearch.IsValid())
	{
		// 2. 검색 조건 설정
		SessionSearch->MaxSearchResults = 10;
		SessionSearch->bIsLanQuery = false; // Steam 사용 시 false
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals); // Presence 사용하는 세션만 검색
		
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UMumulGameInstance::JoinGameSession(int32 SessionIndex)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid() || !SessionSearch->SearchResults.IsValidIndex(SessionIndex))
	{
		UE_LOG(LogTemp, Error, TEXT("JoinGameSession failed: Invalid SessionInterface, SessionSearch, or Index."));
		return;
	}

	// 1. 선택한 세션 결과 가져오기
	const FOnlineSessionSearchResult& SearchResult = SessionSearch->SearchResults[SessionIndex];

	// 2. 세션 참여 시도 (호출자 ID: 0)
	// OnJoinSessionComplete가 바인딩되어 있습니다.
	SessionInterface->JoinSession(0, RequestedSessionName, SearchResult); // 세션 참여 시 RequestedSessionName을 임시 세션 이름으로 사용
}

void UMumulGameInstance::TravelToLevel(const FString& LevelName)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("World is NULL. Cannot travel."));
		return;
	}
	
	// 호스트 (세션 생성자)의 경우: ?listen 옵션을 붙여서 리스닝 서버로 맵 이동
	if (LevelName.Contains("?listen"))
	{
		World->ServerTravel(LevelName, false);
	}
	// 클라이언트 (세션 참여자)의 경우: 이미 OnJoinSessionComplete에서 ClientTravel을 사용했으므로,
	// 이 함수는 주로 호스트가 맵을 로드할 때 사용됨.
	else
	{
		// 클라이언트는 서버 접속 문자열을 통해 ClientTravel을 사용해야 함.
		// 일반적인 레벨 이동은 ServerTravel 또는 ClientTravel을 사용하지만, 
		// 멀티플레이어 환경에서는 **ServerTravel**이 주로 사용됨.
		UE_LOG(LogTemp, Warning, TEXT("Attempting ServerTravel to: %s"), *LevelName);
		World->ServerTravel(LevelName, false);
	}
}

void UMumulGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnCreateSessionComplete: %s, Success: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	OnSessionCreated.Broadcast(bWasSuccessful);

	// 세션 생성 성공 시 바로 레벨 이동 (리스닝 서버 시작)
	if (bWasSuccessful)
	{
		TravelToLevel(RequestedTravelURL);
	}
}

void UMumulGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnFindSessionsComplete. Success: %s. Found Sessions: %d"), 
		bWasSuccessful ? TEXT("true") : TEXT("false"), 
		SessionSearch.IsValid() ? SessionSearch->SearchResults.Num() : 0);

	if (bWasSuccessful && SessionSearch.IsValid())
	{
		// 검색 결과 처리 (UI에 표시 등)
		for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
		{
			FString SessionId = Result.Session.GetSessionIdStr();
			FString HostUsername = Result.Session.OwningUserName;
			int32 CurrentPlayers = Result.Session.NumOpenPublicConnections;

			UE_LOG(LogTemp, Warning, TEXT("Found Session: ID=%s, Host=%s, Players=%d"), 
				*SessionId, *HostUsername, CurrentPlayers);
			
			// 실제 UI 처리 로직은 여기서 구현해야 합니다.
		}
	}
	
	// 검색 결과 없음 등의 후속 처리
}

void UMumulGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		UE_LOG(LogTemp, Error, TEXT("JoinSession failed. Result: %d"), (int32)Result);
		return;
	}

	// 1. 접속 정보 (Connect String) 가져오기
	FString ConnectString;
	if (!SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get resolved connect string."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("JoinSession Success! Connect String: %s"), *ConnectString);

	// 2. 접속 문자열을 사용하여 레벨 이동 (클라이언트로서 서버에 접속)
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
	}
}

void UMumulGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnDestroySessionComplete: %s, Success: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	
	// 세션 파괴 성공 시, 미리 저장해 둔 정보로 세션 재생성 시도
	if (bWasSuccessful)
	{
		// 요청 정보로 세션 재생성
		InternalCreateSession(RequestedSessionName, 4, false, RequestedTravelURL); // MaxPlayers와 bIsLAN은 적절한 값으로 대체 필요
	}
	else
	{
		// 파괴 실패 시 생성도 못함
		OnSessionCreated.Broadcast(false);
	}
}

void UMumulGameInstance::InternalCreateSession(FName SessionName, int32 MaxPlayers, bool bIsLAN, FString TravelURL)
{
	if (!SessionInterface.IsValid())
	{
		OnSessionCreated.Broadcast(false);
		return;
	}

	// 1. 세션 설정 객체 생성
	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	
	// 2. 기본 설정
	SessionSettings->bIsLANMatch = bIsLAN;
	SessionSettings->NumPublicConnections = MaxPlayers;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true; // Steam 오버레이 및 친구 초대에 필요
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bIsDedicated = false; // 전용 서버가 아닌 리스닝 서버
	
	// 3. 사용자 정의 속성 추가 (검색 필터링에 사용)
	// 예: 게임 모드 이름
	SessionSettings->Set(
		FName(TEXT("GAMEMODE_KEY")), 
		FString(TEXT("DeathMatch")), 
		EOnlineDataAdvertisementType::ViaOnlineService
	);
	
	// 4. 세션 생성 시도
	bool bSuccess = SessionInterface->CreateSession(0, SessionName, *SessionSettings);
	
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSession failed to start."));
		OnSessionCreated.Broadcast(false);
	}
}
