// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundAttenuation.h"
#include "CuteAlienController.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerArrayUpdated);


USTRUCT()
struct FTeamUser
{
	GENERATED_BODY()

	UPROPERTY()
	int32 UserId = 0;

	UPROPERTY()
	FString UserName;
};


UCLASS()
class MUMUL_API ACuteAlienController : public APlayerController
{
	GENERATED_BODY()
	ACuteAlienController();
	
public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY()
	TObjectPtr<class AMumulGameState> GS;
	UPROPERTY()
	TObjectPtr<class UHttpNetworkSubsystem> HttpSystem;

	UFUNCTION(Server, Reliable)
	void Server_InitPlayerInfo(int32 UID, const FString& Name, const FString& Type, int32 Tendency);


	UPROPERTY()
	TObjectPtr<class UInputMappingContext> IMC_Player;

public:
	virtual void Tick(float DeltaSeconds) override;

	// ESC 키에 바인딩할 함수
	// 저장 후 로비로 가거나 게임 종료
	void SaveAndExit();
	

	
	// Component
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UPlayerHousingSystemComponent> HousingComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UPlayerChatComponent> ChatComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UPlayerMeetingManagerComponent> MeetingComp;

public:
	UPROPERTY()
	TObjectPtr<class UInputAction> IA_Radial;
	UPROPERTY()
	TObjectPtr<class UInputAction> IA_Cancel;
	void OnCancelUI();
	UPROPERTY()
	TObjectPtr<class UInputAction> IA_ToggleMouse;
	void OnToggleMouse();
	
	UPROPERTY()
	TObjectPtr<class UInputAction> IA_Click;
	void OnClick(const FVector& TentLocation, const FRotator& TentRotation);
	
	UPROPERTY()
	TObjectPtr<class UInputAction> IA_QuitGame;
	void OnPressEsc();
	UPROPERTY()
	TObjectPtr<class UInputAction> IA_Interact;
	void OnInteract();
public:
	UFUNCTION(Server, Reliable)
	void Server_RequestStartQuiz(class AOXQuizTriggerActor* QuizTrigger);

protected:
	UPROPERTY()
	TSubclassOf<class URadialUI> RadialUIClass;
	UPROPERTY()
	TObjectPtr<URadialUI> RadialUI;

	bool bIsRadialVisible = false;

	void ShowRadialUI();
	void HideRadialUI();
	void CancelRadialUI();

	UPROPERTY()
	TSubclassOf<class UPlayerUI> PlayerUIClass;
	// UPROPERTY()
	// TSubclassOf<class UGroupChatUI> GroupChatUIClass;
public:
	UPROPERTY()
	TObjectPtr<UPlayerUI> PlayerUI;
	// UPROPERTY()
	// TObjectPtr<UGroupChatUI> GroupChatUI;
	UPROPERTY()
	TObjectPtr<class UIMGManager> IMGManager;

protected:
	// PlayerState 초기화 대기용 타이머 핸들
	FTimerHandle InitPlayerStateTimerHandle;

	// 타이머에 의해 호출될 초기화 함수
	void TryInitPlayerInfo();
	
// 	UPROPERTY()
// 	TSubclassOf<class UGroupIconUI> GroupIconUIClass;
// 	UFUNCTION()
// 	void OnServerCreateTeamChatResponse(bool bSuccess, FString Message);
//
// public:
// 	UFUNCTION(Server, Reliable)
// 	void Server_AddTeamChatList(const FString& TeamID);
//

//
// 	UFUNCTION(Server, Reliable)
// 	void Server_RequestTeamChatList();
// 	UFUNCTION(NetMulticast, Reliable)
// 	void Multicast_RequestTeamChatList();
//
// 	UFUNCTION(Server, Reliable)
// 	void Server_CreateGroupChatUI(const TArray<int32>& UserIDs, const FString& TeamID, const FString& TeamName,
// 	                              const TArray<FTeamUser>& TeamUserIDs);
// 	UFUNCTION(Client, Reliable)
// 	void Client_CreateGroupChatUI(const FString& TeamID, const FString& TeamName,
// 	                              const TArray<FTeamUser>& TeamUserIDs, UTexture2D* IMG);
//
// 	UFUNCTION(Server, Reliable)
// 	void Server_RequestChat(const FString& TeamID, const TArray<int32>& UserIDs, const FString& CurrentTime,
// 	                         const int32& UserID, const FString& Name, const FString& Text);
// 	UFUNCTION(Client, Reliable)
// 	void Client_SendChat(const FString& TeamID, const FString& CurrentTime, const int32& UserID, const FString& Name, const FString& Text);

protected:
	UPROPERTY()
	TSubclassOf<class UOXQuizUI> OXQuizUIClass;
	UPROPERTY()
	TObjectPtr<class UOXQuizUI> OXQuizUI;
public:
	UFUNCTION(Client, Reliable)
	void Client_DisplayQuestion(const int32& QuestionIdx, const FString& NewQuestion, const int32& QuestionTime);
	UFUNCTION(Client, Reliable)
	void Client_DisplayAnswer(bool AnswerResult, bool NewAnswer, const FString& NewCommentary, const int32& AnswerTime);
	UFUNCTION(Client, Reliable)
	void Client_DisplayResult(const int32& QuestionIdx, bool AnswerResult, const FString& QuestionText, bool AnswerText, const FString& CommentaryText);
};
