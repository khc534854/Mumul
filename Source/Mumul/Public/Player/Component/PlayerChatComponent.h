// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerChatComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MUMUL_API UPlayerChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPlayerChatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY()
	TSubclassOf<class UGroupChatUI> GroupChatUIClass;
	UPROPERTY()
	TObjectPtr<UGroupChatUI> GroupChatUI;
	
	UPROPERTY()
	TSubclassOf<class UGroupIconUI> GroupIconUIClass;
	UFUNCTION()
	void OnServerCreateTeamChatResponse(bool bSuccess, FString Message);

	UFUNCTION(Server, Reliable)
	void Server_AddTeamChatList(const FString& TeamID);



	UFUNCTION(Server, Reliable)
	void Server_RequestTeamChatList();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RequestTeamChatList();

	UFUNCTION(Server, Reliable)
	void Server_CreateGroupChatUI(const TArray<int32>& UserIDs, const FString& TeamID, const FString& TeamName,
								  const TArray<FTeamUser>& TeamUserIDs);
	UFUNCTION(Client, Reliable)
	void Client_CreateGroupChatUI(const FString& TeamID, const FString& TeamName,
								  const TArray<FTeamUser>& TeamUserIDs, UTexture2D* IMG);

	UFUNCTION(Server, Reliable)
	void Server_RequestChat(const FString& TeamID, const TArray<int32>& UserIDs, const FString& CurrentTime,
							 const int32& UserID, const FString& Name, const FString& Text);
	UFUNCTION(Client, Reliable)
	void Client_SendChat(const FString& TeamID, const FString& CurrentTime, const int32& UserID, const FString& Name, const FString& Text);
	
	class ACuteAlienController* owner;
	class ACuteAlienPlayer* player;
};
