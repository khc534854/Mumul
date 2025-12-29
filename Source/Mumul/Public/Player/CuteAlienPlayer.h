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

	virtual void OnRep_PlayerState() override;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UWidgetInteractionComponent> UIInteractionComp;
	UPROPERTY()
	TObjectPtr<class UInputAction> IA_Click;
	void OnClickInteraction();
	
public:
	void UpdateVoiceIconState();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void UpdateBodyMaterial(int32 TendencyIdx);

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

	UPROPERTY()
	TObjectPtr<class UAnimMontage> SitMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Customs")
	UStaticMeshComponent* CustomMeshComponent;
	
public:
	UFUNCTION(Server, Reliable)
	void Server_PlayAlienDance(int32 SelectIdx);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAlienDance(int32 SelectIdx);

	UFUNCTION(Server, Reliable)
	void Server_SitDown();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SitDown();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UVoiceChatComponent* VoiceComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UWidgetComponent* WidgetComponent;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void UpdateNameTag();

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
	UPROPERTY(EditDefaultsOnly)
	TArray<UMaterialInterface*> PlayerBodyMaterials;
	
	UFUNCTION(Server, Reliable)
	void Server_EquipCustom(FName ItemID);

	void UpdateCustomMesh(FName ItemID);
	
	UPROPERTY(EditAnywhere, Category="Cloud|FX")
	TObjectPtr<class UNiagaraSystem> LightningBoltVFX;
	UPROPERTY(EditAnywhere, Category="Cloud|FX")
	TObjectPtr<class UNiagaraSystem> LightningImpactVFX;
	UPROPERTY(EditAnywhere, Category="Montage")
	TObjectPtr<class UAnimMontage> ElectrocutedMontage;

	UFUNCTION(Server, Reliable)
	void Server_PlayElectrocutedMontage(FVector FireLocation,
	FVector FireDirection);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayElectrocutedMontage(FVector FireLocation,
	FVector FireDirection);
	
	UPROPERTY(EditDefaultsOnly, Category="Sound")
	TObjectPtr<USoundBase> ElectricShock;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundBase> Zip;
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<class USoundBase> Boing;
	UFUNCTION()
	void OnSoundFinished();
public:
	void PlayTentSpawnSound();

	void SetIsMeetingSitting(bool bIsSitting, AActor* FocusTarget = nullptr);

	// [신규] 일어서기 몽타주 (있다면)
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TObjectPtr<class UAnimMontage> StandUpMontage;

	UFUNCTION(Server, Reliable)
	void Server_StandUp();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StandUp();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SitAtLocation(FVector TargetLoc, FRotator TargetRot, ACampFireActor* TargetFire);

	void UpdateNameTagVisibility();

	float NameTagVisibleDistance = 1500.0f;
private:
	UPROPERTY()
	USceneComponent* OriginalCameraParent = nullptr;
    
	FTransform OriginalCameraTransform;
	float OriginalArmLength = 0.0f;
	FVector OriginalSocketOffset;
};
