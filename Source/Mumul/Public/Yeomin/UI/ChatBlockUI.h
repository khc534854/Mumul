// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatBlockUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UChatBlockUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	TArray<int32> PlayersInGroup;
	
public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> ChatScrollBox;
	void SetPlayersInGroup(const TArray<int32>& Players) { PlayersInGroup = Players; }
	TArray<int32>& GetPlayersInGroup() { return PlayersInGroup; }
};
