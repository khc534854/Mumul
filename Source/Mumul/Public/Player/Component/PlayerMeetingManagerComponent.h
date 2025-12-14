// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerMeetingManagerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUMUL_API UPlayerMeetingManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerMeetingManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
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

	UFUNCTION()
	void OnHostRecordingStopped();

	UFUNCTION(Server, Reliable)
	void Server_BroadcastJoinMeeting(const FString& TargetChannelID, const FString& MeetingID);

	UFUNCTION(Client, Reliable)
	void Client_RequestJoinMeeting(const FString& MeetingID);
	
	// 녹음 버튼 클릭 시 호출 (UI 열기)
	UFUNCTION(BlueprintCallable, Category = "Meeting")
	void OpenMeetingSetupUI();

	// 종료 버튼 클릭 시 호출 (팝업 열기)
	UFUNCTION(BlueprintCallable, Category = "Meeting")
	void OpenEndMeetingPopup();
	
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void UpdateVoiceChannelMuting();

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

	UPROPERTY()
	TMap<int32, class UVOIPTalker*> CachedTalkers;
	
	// 무음 처리용 감쇠 설정
	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	TObjectPtr<USoundAttenuation> SilentAttenuation;

	UPROPERTY(EditDefaultsOnly, Category = "Voice")
	TObjectPtr<USoundAttenuation> NormalAttenuation;
	
	class ACuteAlienController* owner;
	class ACuteAlienPlayer* player;
};
