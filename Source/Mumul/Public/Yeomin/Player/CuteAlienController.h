// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundAttenuation.h"
#include "CuteAlienController.generated.h"

/**
 * 
 */

USTRUCT()
struct FTeamUser
{
	GENERATED_BODY()

	UPROPERTY()
	int32 UserId;

	UPROPERTY()
	FString UserName;
};


UCLASS()
class MUMUL_API ACuteAlienController : public APlayerController
{
	GENERATED_BODY()
	ACuteAlienController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY()
	TObjectPtr<class AMumulGameState> GS;

	UFUNCTION(Server, Reliable)
	void Server_InitPlayerInfo(int32 UID, const FString& Name, const FString& Type, int32 Tendency);

	UPROPERTY()
	TObjectPtr<class UInputMappingContext> IMC_Player;

public:
	virtual void Tick(float DeltaSeconds) override;

	// ESC 키에 바인딩할 함수

	// 저장 후 로비로 가거나 게임 종료
	UFUNCTION(Server, Reliable)
	void Server_SaveAndExit();

	UFUNCTION()
	void OnHostRecordingStopped();

protected:
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
	TSubclassOf<class URadialUI> RadialUIClass;
	UPROPERTY()
	TObjectPtr<URadialUI> RadialUI;

	bool bIsRadialVisible = false;

	void ShowRadialUI();
	void HideRadialUI();
	void CancelRadialUI();

	UPROPERTY()
	TSubclassOf<class UPlayerUI> PlayerUIClass;
	UPROPERTY()
	TObjectPtr<UPlayerUI> PlayerUI;

	UPROPERTY()
	TSubclassOf<class UGroupChatUI> GroupChatUIClass;
	UPROPERTY()
	TObjectPtr<UGroupChatUI> GroupChatUI;

	UPROPERTY()
	TSubclassOf<class APreviewTentActor> PreviewTentClass;
	UPROPERTY()
	TObjectPtr<class APreviewTentActor> PreviewTent;
	UPROPERTY()
	TSubclassOf<class ATentActor> TentClass;
	UPROPERTY()
	TObjectPtr<class ATentActor> Tent;

public:
	void ShowPreviewTent();

	UFUNCTION(Server, Reliable)
	void Server_SpawnTent(const FTransform& TentTransform);

	void RequestStartMeetingRecording(FString InMeetingTitle, FString InAgenda, FString InDesc);
	void RequestStopMeetingRecording();

	UFUNCTION(Server, Reliable)
	void Server_StartChannelRecording(const FString& TargetChannelID);

	UFUNCTION(Client, Reliable)
	void Client_StartChannelRecording(const FString& TargetChannelID);

	UFUNCTION(Server, Reliable)
	void Server_StopChannelRecording(const FString& TargetChannelID);

	UFUNCTION(Client, Reliable)
	void Client_StopChannelRecording();


	UFUNCTION(Server, Reliable)
	void Server_BroadcastJoinMeeting(const FString& TargetChannelID, const FString& MeetingID);

	UFUNCTION(Client, Reliable)
	void Client_RequestJoinMeeting(const FString& MeetingID);

public:
	// 녹음 버튼 클릭 시 호출 (UI 열기)
	UFUNCTION(BlueprintCallable, Category = "Meeting")
	void OpenMeetingSetupUI();

	// 종료 버튼 클릭 시 호출 (팝업 열기)
	UFUNCTION(BlueprintCallable, Category = "Meeting")
	void OpenEndMeetingPopup();
	
protected:
	// [변수] 현재 진행 중인 회의 ID (서버에서 받아서 저장)
	FString CurrentMeetingSessionID; 

	// [함수] HTTP 응답 핸들러 (바인딩용)
	UFUNCTION()
	void OnStartMeetingResponse(bool bSuccess, FString MeetingID);

	UFUNCTION()
	void OnJoinMeetingResponse(bool bSuccess);

	UFUNCTION(Server, Reliable)
	void Server_RegisterMeetingState(const FString& ChannelID, const FString& MeetingID);

	// [신규] 서버의 GameState에서 회의 정보 삭제 요청
	UFUNCTION(Server, Reliable)
	void Server_UnregisterMeetingState(const FString& ChannelID);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UVoiceMeetingUI> VoiceMeetingUIClass;

	UPROPERTY()
	TObjectPtr<class UVoiceMeetingUI> VoiceMeetingUI;
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void UpdateVoiceChannelMuting();

protected:
	// 무음 처리용 감쇠 설정
	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	TObjectPtr<USoundAttenuation> SilentAttenuation;

	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	TObjectPtr<USoundAttenuation> NormalAttenuation;
	
	UPROPERTY()
	TSubclassOf<class UGroupIconUI> GroupIconUIClass;
	UFUNCTION()
	void OnServerCreateTeamChatResponse(bool bSuccess, FString Message);

public:
	UFUNCTION(Server, Reliable)
	void Server_AddTeamChatList(const FString& TeamID);
	
	UPROPERTY()
	TObjectPtr<class UIMGManager> IMGManager;
	
	UFUNCTION(Server, Reliable)
	void Server_CreateGroupChatUI(const TArray<int32>& UserIDs, const FString& TeamID, const FString& TeamName,
								  const TArray<FTeamUser>& TeamUserIDs);
	UFUNCTION(Client, Reliable)
	void Client_CreateGroupChatUI(const FString& TeamID, const FString& TeamName,
	                              const TArray<FTeamUser>& TeamUserIDs, UTexture2D* IMG);

	UFUNCTION(Server, Reliable)
	void Server_RequestChat(const FString& TeamID, const TArray<int32>& UserIDs, const FString& CurrentTime,
	                        const FString& Name, const FString& Text);
	UFUNCTION(Client, Reliable)
	void Client_SendChat(const FString& TeamID, const FString& CurrentTime, const FString& Name, const FString& Text);
};
