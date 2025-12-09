// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SessionInfoWidget.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API USessionInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* textSessionName;
	
	UPROPERTY(meta=(BindWidget))
	class UButton* btn_Join;

	int32 sessionidx;
	//정보 설정 함수
	void SetSessionInfo(int32 idx , FString sessionName);

	//참여 버튼 클릭함수
	UFUNCTION()
	void OnClickJoin();
	
};
