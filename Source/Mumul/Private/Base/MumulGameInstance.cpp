// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/MumulGameInstance.h"
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
	
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
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
	if (!SessionInterface.IsValid()) return;

	RequestedSessionName = FName(*SessionName);
	RequestedTravelURL = TravelURL;
	bIsCreatingSession = true; // [추가] "나 지금 방 만들려고 하는 중이야" 표시

	auto ExistingSession = SessionInterface->GetNamedSession(RequestedSessionName);
	if (ExistingSession)
	{
		SessionInterface->DestroySession(RequestedSessionName);
	}
	else
	{
		InternalCreateSession(RequestedSessionName, MaxPlayers, bIsLAN);
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
		SessionSearch->MaxSearchResults = 50; // 10개는 너무 적음 (다른 사람 방이 많으므로)
		SessionSearch->bIsLanQuery = false;
        
		// Presence 검색 활성화
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
        

		SessionSearch->QuerySettings.Set(
		FName(TEXT("MUMUL_MATCH_KEY")),
		FString(TEXT("MUMUL_SESSION")),
		EOnlineComparisonOp::Equals
		);

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

	// 중요: 클라이언트는 RequestedSessionName 변수가 비어있으므로 NAME_GameSession 사용
	SessionInterface->JoinSession(0, NAME_GameSession, SearchResult);
}

void UMumulGameInstance::TravelToLevel(const FString& LevelName)
{
	UWorld* World = GetWorld();
	if (!World) return;

	World->ServerTravel(LevelName, true);
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
	OnFindSessionsCompleteEvent.Broadcast(bWasSuccessful);
	
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
	if (Result != EOnJoinSessionCompleteResult::Success) return;

	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		// [수정] 접속 주소 뒤에 내 아이디를 옵션으로 붙임
		// 결과 예시: "123.456.78.9:7777?Name=user1"
		FString TravelURL = FString::Printf(TEXT("%s?Name=%d"), *ConnectString, PlayerUniqueID);

		UE_LOG(LogTemp, Log, TEXT("Traveling to: %s"), *TravelURL);

		if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute);
		}
	}
}

void UMumulGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnDestroySessionComplete: %s, Success: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	if (GetWorld()->GetNetDriver())
	{
		GetWorld()->GetNetDriver()->Shutdown();
	}
	FGenericPlatformMisc::RequestExit(false);
	
	if (bWasSuccessful && bIsCreatingSession) // [추가] 방 만드는 중이었을 때만 재생성
	{
		InternalCreateSession(RequestedSessionName, 20, false); 
		bIsCreatingSession = false; // [추가] 플래그 초기화 (중요)
	}
	else
	{
		// 그냥 방이 파괴된 경우 (게임 종료 등)
		bIsCreatingSession = false;
	}
}

void UMumulGameInstance::InternalCreateSession(FName SessionName, int32 MaxPlayers, bool bIsLAN)
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
	SessionSettings->bUseLobbiesIfAvailable = true;
	
	
	// 3. 사용자 정의 속성 추가 (검색 필터링에 사용)
	// 예: 게임 모드 이름
	SessionSettings->Set(
		FName(TEXT("MUMUL_MATCH_KEY")), 
		FString(TEXT("MUMUL_SESSION")), 
		EOnlineDataAdvertisementType::ViaOnlineServiceAndPing
	);
	
	// 4. 세션 생성 시도
	bool bSuccess = SessionInterface->CreateSession(0, SessionName, *SessionSettings);
	
	if (!bSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSession failed to start."));
		OnSessionCreated.Broadcast(false);
	}
}

FString UMumulGameInstance::GetSteamNickname()
{
	IOnlineSubsystem* onlineSubsystem = Online::GetSubsystem(GetWorld());
	if (!onlineSubsystem) return TEXT("Unknown");

	IOnlineIdentityPtr Identity = onlineSubsystem->GetIdentityInterface();
	if (!Identity.IsValid()) return TEXT("Unknown");
	
	FString Nickname = Identity->GetPlayerNickname(0);
	if (!onlineSubsystem->GetSubsystemName().IsEqual("STEAM"))
	{
		Nickname = TEXT("Local_") + Nickname.Left(5);
	}
	return Nickname.IsEmpty() ? TEXT("Unknown") : Nickname;
}
