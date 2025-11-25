// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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

protected:
	UPROPERTY()
	TSubclassOf<class UUserWidget> PlayerUIClass;
	UPROPERTY()
	TObjectPtr<UUserWidget> PlayerUI;

protected:
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

public:
	UFUNCTION(BlueprintCallable, Category = "Voice")
	void UpdateVoiceChannelMuting();
	
	UFUNCTION(Server, Reliable)
	void Server_SpawnTent(const FTransform& TentTransform);
	void Server_MoveTent(const FTransform& TentTransform);
};
