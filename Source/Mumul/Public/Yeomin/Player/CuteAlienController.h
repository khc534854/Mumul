// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundAttenuation.h"
#include "CuteAlienController.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API ACuteAlienController : public APlayerController
{
	GENERATED_BODY()
	ACuteAlienController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void Server_InitPlayerInfo(int32 UID, const FString& Name, const FString& Type, int32 Tendency);

	UPROPERTY()
	TObjectPtr<class UInputMappingContext> IMC_Player;

public:
	virtual void Tick(float DeltaSeconds) override;

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

	void RequestStartMeetingRecording();
	void RequestStopMeetingRecording();

	UFUNCTION(Server, Reliable)
	void Server_StartChannelRecording(int32 TargetChannelID);

	UFUNCTION(Client, Reliable)
	void Client_StartChannelRecording(int32 TargetChannelID);

	UFUNCTION(Server, Reliable)
	void Server_StopChannelRecording(int32 TargetChannelID);

	UFUNCTION(Client, Reliable)
	void Client_StopChannelRecording();
	
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
public:
	UFUNCTION(Server, Reliable)
	void Server_RequestGroupChatUI(const FString& GroupName, const TArray<int32>& Players);
	UFUNCTION(Client, Reliable)
	void Client_CreateGroupChatUI(const FString& GroupName, const TArray<int32>& Players);

	UFUNCTION(Server, Reliable)
	void Server_RequestChat(const TArray<int32>& Players, const FString& CurrentTime, const FString& Name, const FString& Text);
	UFUNCTION(Client, Reliable)
	void Client_SendChat(const FString& CurrentTime, const FString& Name, const FString& Text);
};
