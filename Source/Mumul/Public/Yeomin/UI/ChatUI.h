// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UChatUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> ChatScrollBox;
};
