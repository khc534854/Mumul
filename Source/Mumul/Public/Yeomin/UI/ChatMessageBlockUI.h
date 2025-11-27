// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatMessageBlockUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UChatMessageBlockUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TextContent;
public:
	void SetContent(FString Content);
};
