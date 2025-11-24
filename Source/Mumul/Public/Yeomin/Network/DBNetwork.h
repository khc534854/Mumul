// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBNetwork.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UDBNetwork : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// POST (데이터 생성)
	UFUNCTION(BlueprintCallable)
	void SendCreateUserRequest(const FString& UserName, int32 Score);

	// GET (데이터 조회)
	UFUNCTION(BlueprintCallable)
	void GetAllData();
	UFUNCTION(BlueprintCallable)
	void GetUserData(const FString& UserID);
	
private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};
