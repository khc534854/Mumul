// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/PlayerChatComponent.h"

#include "Base/MumulGameInstance.h"
#include "Base/MumulGameState.h"
#include "Blueprint/UserWidget.h"
#include "Data/IMGManager.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Network/NetworkStructs.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "UI/ChatBlockUI.h"
#include "UI/GroupChatUI.h"
#include "UI/GroupIconUI.h"
#include "UI/PlayerUI.h"


// Sets default values for this component's properties
UPlayerChatComponent::UPlayerChatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
	static ConstructorHelpers::FClassFinder<UGroupChatUI> GroupChatUIFinder(
	TEXT("/Game/Yeomin/Characters/UI/BP/WBP_GroupChatUI.WBP_GroupChatUI_C"));
	if (GroupChatUIFinder.Succeeded())
	{
		GroupChatUIClass = GroupChatUIFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UGroupIconUI> GroupIconUIFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/WBP_GroupProfileUI.WBP_GroupProfileUI_C"));
	if (GroupIconUIFinder.Succeeded())
	{
		GroupIconUIClass = GroupIconUIFinder.Class;
	}
}


// Called when the game starts
void UPlayerChatComponent::BeginPlay()
{
	Super::BeginPlay();
    
	owner = Cast<ACuteAlienController>(GetOwner());
	if (owner)
		player = Cast<ACuteAlienPlayer>(owner->GetPawn()); 

	// [중요] 내 컨트롤러일 때만 UI와 서브시스템을 초기화해야 합니다.
	if (owner && owner->IsLocalController()) 
	{
		// 1. 서브시스템 바인딩
		if (UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetWorld()->GetGameInstance()))
		{
			if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
			{
				HttpSystem->OnCreateTeamChatResponse.AddDynamic(this, &UPlayerChatComponent::OnServerCreateTeamChatResponse);
			}
		}

		// 2. 위젯 생성
		if (GroupChatUIClass)
		{
			GroupChatUI = CreateWidget<UGroupChatUI>(owner, GroupChatUIClass);
			if (GroupChatUI)
			{
				GroupChatUI->AddToViewport();
              
				if (owner->PlayerUI)
				{
					owner->PlayerUI->InitGroupChatUI(GroupChatUI);
				}
			}
		}
	}
}


// Called every frame
void UPlayerChatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerChatComponent::OnServerCreateTeamChatResponse(bool bSuccess, FString Message)
{
	if (bSuccess)
	{
		// 1. JSON 파싱 (Message에는 JSON 원본이 들어있음)
		FCreateTeamChatResponse CreateTeamChat;

		if (FJsonObjectConverter::JsonObjectStringToUStruct(Message, &CreateTeamChat, 0, 0))
		{
			// // JSON Parsing LOG
			// UE_LOG(LogTemp, Warning, TEXT("===== CreateTeamChat Response ====="));
			// UE_LOG(LogTemp, Warning, TEXT("groupId: %s"), *CreateTeamChat.groupId);
			// UE_LOG(LogTemp, Warning, TEXT("groupName: %s"), *CreateTeamChat.groupName);
			//
			// UE_LOG(LogTemp, Warning, TEXT("userIdList (%d명):"), CreateTeamChat.userIdList.Num());
			// for (int32 UserID : CreateTeamChat.userIdList)
			// {
			// 	UE_LOG(LogTemp, Warning, TEXT(" - userId: %d"), UserID);
			// }
			//
			// TArray<FTeamUser> TeamUserIDs;
			//
			// FTeamData NewTeamData;
			// NewTeamData.UniqueTeamID = CreateTeamChat.groupId;
			// NewTeamData.TeamName = CreateTeamChat.groupName;
			// NewTeamData.TeamMateList = CreateTeamChat.userIdList;
			//
			// for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
			// {
			// 	if (AMumulPlayerState* MPS = Cast<AMumulPlayerState>(PS))
			// 	{
			// 		MPS->PS_PlayerTeamList.Add(NewTeamData);
			// 		if (CreateTeamChat.userIdList.Contains(MPS->PS_UserIndex))
			// 		{
			// 			FTeamUser NewUser;
			// 			NewUser.UserId = MPS->PS_UserIndex;
			// 			NewUser.UserName = MPS->PS_RealName;
			// 			TeamUserIDs.Add(NewUser);
			// 		}
			// 	}
			// }

			if (owner->IsLocalController())
			{
				Server_RequestTeamChatList();
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("CreateTeamChat 파싱 실패"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CreateTeamChat Response 실패 : %s"), *Message);
	}
}

void UPlayerChatComponent::Server_AddTeamChatList_Implementation(const FString& TeamID)
{
	if (AMumulGameState* GS = Cast<AMumulGameState>(GetWorld()->GetGameState()))
	{
		GS->AddTeamChatList(TeamID);
	}
}

void UPlayerChatComponent::Server_RequestTeamChatList_Implementation()
{
	Multicast_RequestTeamChatList();
}

void UPlayerChatComponent::Multicast_RequestTeamChatList_Implementation()
{
	// Get TeamChatList
	if (UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetWorld()->GetGameInstance()))
	{
		if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			if (AMumulPlayerState* PS = owner->GetPlayerState<AMumulPlayerState>())
			{
				HttpSystem->SendTeamChatListRequest(PS->PS_UserIndex);
			}
		}
	}
}

void UPlayerChatComponent::Server_CreateGroupChatUI_Implementation(const TArray<int32>& UserIDs, const FString& TeamID,
	const FString& TeamName, const TArray<FTeamUser>& TeamUserIDs)
{
	if (owner->IMGManager)
	{
		UTexture2D* TeamIconIMG = owner->IMGManager->GetImageByTeamID(TeamID);

		// Add GroupChatUI for each Client
		for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
		{
			if (UserIDs.Contains(Cast<AMumulPlayerState>(PS)->PS_UserIndex))
			{
				if (ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController()))
				{
					PC->ChatComp->Client_CreateGroupChatUI(TeamID, TeamName, TeamUserIDs, TeamIconIMG);
				}
			}
		}
	}
}

void UPlayerChatComponent::Client_CreateGroupChatUI_Implementation(const FString& TeamID, const FString& TeamName,
	const TArray<FTeamUser>& TeamUserIDs, UTexture2D* IMG)
{
	if (!owner || !GroupIconUIClass || !GroupChatUI) return;
	
	// Set Players in Group Icon
	UGroupIconUI* GroupIconUI = CreateWidget<UGroupIconUI>(GetWorld(), GroupIconUIClass);
	if (GroupIconUI)
	{
		GroupChatUI->AddGroupIcon(GroupIconUI);
		GroupIconUI->InitParentUI(GroupChatUI);
		GroupIconUI->ChatBlockUI->SetTeamID(TeamID);
		GroupIconUI->ChatBlockUI->SetTeamName(TeamName);
        
		for (const FTeamUser& User : TeamUserIDs)
		{
			GroupIconUI->ChatBlockUI->AddTeamUser(User.UserId, User.UserName);
		}
		GroupIconUI->SetIconIMG(IMG);
	}
}

void UPlayerChatComponent::Server_RequestChat_Implementation(const FString& TeamID, const TArray<int32>& UserIDs,
	const FString& CurrentTime, const int32& UserID, const FString& Name, const FString& Text)
{
	// Add GroupChatUI for each Client
	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
	{
		if (UserIDs.Contains(Cast<AMumulPlayerState>(PS)->PS_UserIndex))
		{
			if (ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController()))
			{
				PC->ChatComp->Client_SendChat(TeamID, CurrentTime, UserID, Name, Text);
			}
		}
	}
}

void UPlayerChatComponent::Client_SendChat_Implementation(const FString& TeamID, const FString& CurrentTime,
	const int32& UserID, const FString& Name, const FString& Text)
{
	GroupChatUI->AddChat(TeamID, CurrentTime, UserID, Name, Text);
}

