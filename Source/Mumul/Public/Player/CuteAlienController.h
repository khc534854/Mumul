// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelSequencePlayer.h"
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

	UFUNCTION(Client, Reliable)
	void Client_PlayLoadSequence();

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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UPlayerOXQuizComponent> OXQuizComp;

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

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UFeedbackUI> FeedbackUIClass;

	UPROPERTY()
	TObjectPtr<UFeedbackUI> FeedbackUI;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class ULogoutUI> LogoutUIClass;

	UPROPERTY()
	TObjectPtr<ULogoutUI> LogoutUI;
public:
	bool bCanInteract = false;
	bool bCanSit = false;
	bool bIsSitting = false;
	
	UPROPERTY()
	TObjectPtr<UPlayerUI> PlayerUI;

	UPROPERTY()
	TObjectPtr<class UIMGManager> IMGManager;

	void TryInteractWithFeedbackActor();
	void SitState(bool newSitState);

	UPROPERTY()
	TObjectPtr<class AChairObject> TargetChair;
	
	void OpenLogoutUI();
protected:
	// PlayerState 초기화 대기용 타이머 핸들
	FTimerHandle InitPlayerStateTimerHandle;

	// 타이머에 의해 호출될 초기화 함수
	void TryInitPlayerInfo();
	
public:
	UPROPERTY()
	TObjectPtr<class UAudioManager> AudioManager;

private:
	FTimerHandle PCGWaitTimerHandle;

	// PCG 완료 체크 함수
	void CheckPCGAndPlayIntro();

	// 시네마틱이 끝났을 때 호출될 함수
	UFUNCTION() 
	void OnIntroSequenceFinished();
    
	// 재생할 시퀀스 플레이어 저장용
	UPROPERTY()
	ULevelSequencePlayer* IntroSequencePlayer;
};
