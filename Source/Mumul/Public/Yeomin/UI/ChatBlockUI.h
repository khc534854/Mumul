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
	FString GroupName;
	TArray<int32> PlayersInGroup;
	
public:
	void SetGroupName(FString Name) { GroupName = Name; }
	FString GetGroupName() { return GroupName; }
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> ChatScrollBox;
	void SetPlayersInGroup(const TArray<int32>& Players) { PlayersInGroup = Players; }
	TArray<int32>& GetPlayersInGroup() { return PlayersInGroup; }
};
