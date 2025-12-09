// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GroupProfileUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UGroupProfileUI : public UUserWidget
{
	GENERATED_BODY()
	
	
protected:
	int32 UserIndex;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UTextBlock> PlayerNameText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UCheckBox> JoinedStateBox;
	
public:
	void SetUserIndex(const int32 Index) { UserIndex = Index; }
	int32 GetUserIndex() const { return UserIndex; }
	void SetPlayerName(FString Name);
	FString GetPlayerName();
	bool GetCheckBoxState();
	
};
