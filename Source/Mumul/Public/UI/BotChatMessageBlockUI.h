// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BotChatMessageBlockUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UBotChatMessageBlockUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> TextContent;
	//UPROPERTY(meta=(BindWidget))
	//TObjectPtr<class URichTextBlock> TextContent;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> PlayerName;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> Time;
	FString ChatID;
	int32 UserID;
	
public:
	void SetContent(const FString& CurrentTime, const FString& Name, const FString& Content) const;
	void SetChatID(const FString& ID) { ChatID = ID; }
	void SetUserID(const int32& ID) { UserID = ID; }
};
