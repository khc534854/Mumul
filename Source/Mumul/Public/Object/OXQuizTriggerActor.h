// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OXQuizTriggerActor.generated.h"

UENUM(BlueprintType)
enum class EQuizDifficulty : uint8
{
	Beginner UMETA(DisplayName = "초급"),
	Intermediate UMETA(DisplayName = "중급"),
	Advanced UMETA(DisplayName = "고급")
};

UCLASS()
class MUMUL_API AOXQuizTriggerActor : public AActor
{
	GENERATED_BODY()
	AOXQuizTriggerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditDefaultsOnly, Category="Mesh")
	TObjectPtr<class UStaticMeshComponent> GlassMesh;
	UPROPERTY()
	TObjectPtr<class UMaterialInstanceDynamic> GlassMID;
	
	float UIRotDistance = 750.f;
	float LookAtDistance = 555.f;
	
	float PhaseOffset;        // 위상 차이
	float AmplitudeScale;     // 진폭 미세 차이
	UPROPERTY(EditAnywhere, Category="Hover")
	float HoverAmplitude = 25.f;
	UPROPERTY(EditAnywhere, Category="Hover")
	float HoverSpeed = 0.95f;
	FVector OriginalLocation;
	float Time;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USceneComponent> SceneComp;
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class UWidgetComponent> DifficultyBubble;

	UPROPERTY(EditAnywhere, Category="Quiz Level")
	EQuizDifficulty QuizDifficulty = EQuizDifficulty::Beginner;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USphereComponent> TriggerSphere;
	UFUNCTION()
	void OnBeginOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                          const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	TObjectPtr<class AOXQuizActor> OXQuizActor;

public:
	FText GetDifficultyText();
	void OnTriggerQuiz(const int32 UserID);
};
