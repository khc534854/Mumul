// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetworkTestActor.generated.h"

UCLASS()
class MUMUL_API ANetworkTestActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANetworkTestActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// --- [테스트 데이터 설정] ---
	// 에디터에서 값을 바꿔가며 테스트할 수 있게 변수로 노출
	UPROPERTY(EditAnywhere, Category = "Test Data")
	FString TestUserID = TEXT("User_01");

	UPROPERTY(EditAnywhere, Category = "Test Data")
	FString TestRoomID = TEXT("Room_Alpha");

	// --- [테스트 버튼들] ---

	// 1. 단순 로그인 테스트 (JSON 전송)
	UFUNCTION(CallInEditor, Category = "Http Test")
	void TestSendLogin();

	// 2. 로그 데이터 전송 테스트 (JSON 템플릿 함수 사용)
	UFUNCTION(CallInEditor, Category = "Http Test")
	void TestSendLog();

	// 3. 가짜 음성 파일 전송 테스트 (Multipart 전송)
	UFUNCTION(CallInEditor, Category = "Http Test")
	void TestSendMultipartVoice();

private:
	// 테스트용 더미 WAV 데이터를 만드는 함수
	TArray<uint8> GenerateDummyWavData(int32 SizeInBytes);
};
