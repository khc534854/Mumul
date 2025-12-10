// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mumul/MumulCharacter.h"
#include "CuteAlienPlayer.generated.h"

UCLASS()
class MUMUL_API ACuteAlienPlayer : public AMumulCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ACuteAlienPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	UPROPERTY()
	TObjectPtr<class UAnimMontage> DanceMontage1;
	UPROPERTY()
	TObjectPtr<class UAnimMontage> DanceMontage2;
	UPROPERTY()
	TObjectPtr<class UAnimMontage> DanceMontage3;
	UPROPERTY()
	TObjectPtr<class UAnimMontage> DanceMontage4;
	UPROPERTY()
	TObjectPtr<class UAnimMontage> DanceMontage5;
	UPROPERTY()
	TObjectPtr<class UAnimMontage> DanceMontage6;
	UPROPERTY()
	TObjectPtr<class UAnimMontage> DanceMontage7;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customs")
	UStaticMeshComponent* CustomMeshComponent;
	
public:
	UFUNCTION(Server, Reliable)
	void Server_PlayAlienDance(int32 SelectIdx);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAlienDance(int32 SelectIdx);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UVoiceChatComponent* VoiceComponent;

	//Minimap
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USpringArmComponent* MinimapSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USceneCaptureComponent2D* MinimapCapture;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	class UTextureRenderTarget2D* MinimapRenderTarget;

	// Custom Item
public:
	UFUNCTION(Server, Reliable)
	void Server_EquipCustom(FName ItemID);

	void UpdateCustomMesh(FName ItemID);
	
};
