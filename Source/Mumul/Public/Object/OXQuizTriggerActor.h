// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OXQuizTriggerActor.generated.h"

UENUM(BlueprintType)
enum class EQuizDifficulty : uint8
{
	Beginner     UMETA(DisplayName = "초급"),
	Intermediate UMETA(DisplayName = "중급"),
	Advanced     UMETA(DisplayName = "고급")
};

UCLASS()
class MUMUL_API AOXQuizTriggerActor : public AActor
{
	GENERATED_BODY()
	AOXQuizTriggerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Quiz Level")
	EQuizDifficulty QuizDifficulty = EQuizDifficulty::Beginner;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<class USphereComponent> TriggerSphere;
	UFUNCTION()
	void OnBeginOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlapPlayer(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY()
	TObjectPtr<class AOXQuizActor> OXQuizActor;
	
public:
	void OnTriggerQuiz(const int32 UserID);
};
