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
	FString TeamID;
	FString TeamName;
	TMap<int32, FString> TeamUserIDs;
	
public:
	void SetTeamID(FString ID) { TeamID = ID; }
	FString GetTeamID() { return TeamID; }
	void SetTeamName(FString Name) { TeamName = Name; }
	FString GetTeamName() { return TeamName; }
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> ChatScrollBox;
	
	void AddTeamUser(const int32& UserID, const FString& UserName) { TeamUserIDs.Add(UserID, UserName); }
	TMap<int32, FString> GetTeamUsers() { return TeamUserIDs; }
};
