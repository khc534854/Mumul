// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CuteAlienController.h"

#include "Player/CuteAlienPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "UI/RadialUI.h"
#include "Blueprint/UserWidget.h"
#include "InputMappingContext.h"
#include "Base/MumulGameInstance.h"
#include "GameFramework/GameStateBase.h"
#include "Player/MumulPlayerState.h"
#include "Object/Tent/PreviewTentActor.h"
#include "Net/VoiceConfig.h"

#include "Network/HttpNetworkSubsystem.h"
#include "Base/MumulGameState.h"
#include "Components/WidgetSwitcher.h"
#include "Save/MapDataSaveGame.h"
#include "Network/NetworkStructs.h"
#include "Kismet/GameplayStatics.h"
#include "Data/IMGManager.h"
#include "Object/OXQuizTriggerActor.h"
#include "UI/ChatBlockUI.h"
#include "UI/GroupChatUI.h"
#include "UI/GroupIconUI.h"
#include "UI/PlayerUI.h"
#include "UI/OXQuiz/OXQuizUI.h"
#include "Player/Component/PlayerChatComponent.h"
#include "Player/Component/PlayerHousingSystemComponent.h"
#include "Player/Component/PlayerMeetingManagerComponent.h"

ACuteAlienController::ACuteAlienController()
{
	static ConstructorHelpers::FClassFinder<URadialUI> RadialUIFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/WBP_RadialUI.WBP_RadialUI_C"));
	if (RadialUIFinder.Succeeded())
	{
		RadialUIClass = RadialUIFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UPlayerUI> PlayerUIFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/WBP_PlayerUI.WBP_PlayerUI_C"));
	if (PlayerUIFinder.Succeeded())
	{
		PlayerUIClass = PlayerUIFinder.Class;
	}

	// static ConstructorHelpers::FClassFinder<UGroupChatUI> GroupChatUIFinder(
	// 	TEXT("/Game/Yeomin/Characters/UI/BP/WBP_GroupChatUI.WBP_GroupChatUI_C"));
	// if (GroupChatUIFinder.Succeeded())
	// {
	// 	GroupChatUIClass = GroupChatUIFinder.Class;
	// }
	//
	// static ConstructorHelpers::FClassFinder<UGroupIconUI> GroupIconUIFinder(
	// 	TEXT("/Game/Yeomin/Characters/UI/BP/WBP_GroupProfileUI.WBP_GroupProfileUI_C"));
	// if (GroupIconUIFinder.Succeeded())
	// {
	// 	GroupIconUIClass = GroupIconUIFinder.Class;
	// }

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> IMCFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/IMC_Player.IMC_Player"));
	if (IMCFinder.Succeeded())
	{
		IMC_Player = IMCFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_RadialFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_Radial.IA_Radial"));
	if (IA_RadialFinder.Succeeded())
	{
		IA_Radial = IA_RadialFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_CancelFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_Cancel.IA_Cancel"));
	if (IA_CancelFinder.Succeeded())
	{
		IA_Cancel = IA_CancelFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_ToggleMouseFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_ToggleMouse.IA_ToggleMouse"));
	if (IA_ToggleMouseFinder.Succeeded())
	{
		IA_ToggleMouse = IA_ToggleMouseFinder.Object;
	}

	static ConstructorHelpers::FClassFinder<UOXQuizUI> OXQuizUIClassFinder(
		TEXT("/Game/Yeomin/Characters/UI/BP/OXQuiz/WBP_OXQuiz.WBP_OXQuiz_C")); // 경로 확인 필수!
	if (OXQuizUIClassFinder.Succeeded())
	{
		OXQuizUIClass = OXQuizUIClassFinder.Class;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> IA_InteractFinder(
		TEXT("/Game/Yeomin/Characters/Inputs/Actions/IA_Interact.IA_Interact"));
	if (IA_InteractFinder.Succeeded())
	{
		IA_Interact = IA_InteractFinder.Object;
	}
	
	// Component
	HousingComp = CreateDefaultSubobject<UPlayerHousingSystemComponent>(TEXT("HousingComp"));
	HousingComp->SetIsReplicated(true); // RPC 사용 시 필수

	MeetingComp = CreateDefaultSubobject<UPlayerMeetingManagerComponent>(TEXT("MeetingComp"));
	MeetingComp->SetIsReplicated(true);

	ChatComp = CreateDefaultSubobject<UPlayerChatComponent>(TEXT("SocialComp"));
	ChatComp->SetIsReplicated(true);
}

void ACuteAlienController::BeginPlay()
{
	Super::BeginPlay();

	GS = Cast<AMumulGameState>(GetWorld()->GetGameState());

	HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>();


	if (!IsLocalController())
		return;

	IMGManager = NewObject<UIMGManager>(this, UIMGManager::StaticClass());

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(IMC_Player, 0);
	}

	if (RadialUIClass)
	{
		RadialUI = CreateWidget<URadialUI>(this, RadialUIClass);
		if (RadialUI)
		{
			RadialUI->AddToViewport();
			RadialUI->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (PlayerUIClass)
	{
		PlayerUI = CreateWidget<UPlayerUI>(this, PlayerUIClass);
		if (PlayerUI)
		{
			PlayerUI->AddToViewport();
		}
	}

	// if (GroupChatUIClass)
	// {
	// 	GroupChatUI = CreateWidget<UGroupChatUI>(this, GroupChatUIClass);
	// 	if (GroupChatUI)
	// 	{
	// 		GroupChatUI->AddToViewport();
	// 	}
	// }

	if (PlayerUI && ChatComp->GroupChatUI)
	{
		PlayerUI->InitGroupChatUI(ChatComp->GroupChatUI);
		RadialUI->SetVisibility(ESlateVisibility::Hidden);
	}

	if (OXQuizUIClass)
	{
		OXQuizUI = CreateWidget<UOXQuizUI>(this, OXQuizUIClass);
		if (OXQuizUI)
		{
			OXQuizUI->AddToViewport();
			OXQuizUI->SetVisibility(ESlateVisibility::Collapsed);
		}
	}


	TryInitPlayerInfo();

	// [수정] PlayerState가 준비될 때까지 타이머로 확인 (0.5초 간격)
	GetWorldTimerManager().SetTimer(InitPlayerStateTimerHandle, this, &ACuteAlienController::TryInitPlayerInfo, 0.5f,
	                                true);
}

void ACuteAlienController::SetupInputComponent()
{
	Super::SetupInputComponent();
	UEnhancedInputComponent* Input = CastChecked<UEnhancedInputComponent>(InputComponent);

	Input->BindAction(IA_Radial, ETriggerEvent::Started, this, &ACuteAlienController::ShowRadialUI);
	Input->BindAction(IA_Radial, ETriggerEvent::Completed, this, &ACuteAlienController::HideRadialUI);
	Input->BindAction(IA_Cancel, ETriggerEvent::Started, this, &ACuteAlienController::OnCancelUI);
	Input->BindAction(IA_ToggleMouse, ETriggerEvent::Started, this, &ACuteAlienController::OnToggleMouse);
	//Input->BindAction(IA_QuitGame, ETriggerEvent::Started, this, &ACuteAlienController::OnPressEsc);
	Input->BindAction(IA_Interact, ETriggerEvent::Started, this, &ACuteAlienController::OnInteract);
}

void ACuteAlienController::Server_InitPlayerInfo_Implementation(int32 UID, const FString& Name, const FString& Type,
                                                                int32 Tendency)
{
	AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();
	if (PS)
	{
		PS->PS_UserIndex = UID;
		if (IsLocalController())
		{
			PS->OnRep_UserIndex();
		}
		PS->SetPlayerName(Name);
		PS->PS_RealName = Name;
		PS->PS_UserType = Type;
		PS->Server_SetVoiceChannelID(TEXT("Lobby"));
		// PS->CampID = CampID; (인자 추가 시)

		PS->PS_TendencyID = Tendency;
		PS->OnRep_TendencyID();
		

		// 강제 동기화 (선택)
		PS->ForceNetUpdate();

		FString SlotName = TEXT("IslandMapSave");
		if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
		{
			UMapDataSaveGame* LoadInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

			// 해당 유저의 저장된 위치가 있는지 확인
			if (LoadInst)
			{
				// Z축(높이)을 살짝 띄워주는 게 안전합니다 (바닥 끼임 방지)
				if (FTransform* SavedTr = LoadInst->PlayerLocations.Find(UID))
				{
					if (APawn* MyPawn = GetPawn())
					{
						// Z축 보정 (바닥 끼임 방지)
						FVector SafeLoc = SavedTr->GetLocation() + FVector(0, 0, 50.0f);
						SavedTr->SetLocation(SafeLoc);
                   
						MyPawn->SetActorTransform(*SavedTr, false, nullptr, ETeleportType::TeleportPhysics);
                   
						UE_LOG(LogTemp, Warning, TEXT("[Server] Restored User %d Location to %s"), UID, *SafeLoc.ToString());
					}
				}

				if (FName* SavedItemID = LoadInst->PlayerCosmetics.Find(UID))
				{
					PS->EquippedCustomID = *SavedItemID;
					PS->OnRep_EquippedCustomID();
					UE_LOG(LogTemp, Log, TEXT("[Server] Loaded User %d Cosmetic: %s"), UID, *SavedItemID->ToString());
				}
			}
		}
		UE_LOG(LogTemp, Log, TEXT("[Server] PlayerState Initialized: %s (ID: %d)"), *Name, UID);
	}
}


void ACuteAlienController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}


void ACuteAlienController::OnPressEsc()
{
	SaveAndExit();
}

void ACuteAlienController::OnInteract()
{
}

void ACuteAlienController::Server_RequestStartQuiz_Implementation(AOXQuizTriggerActor* QuizTrigger)
{
	QuizTrigger->OnTriggerQuiz(Cast<AMumulPlayerState>(PlayerState)->PS_UserIndex);
}

void ACuteAlienController::SaveAndExit()
{
	// 1. 저장 (아직 폰과 연결되어 있으므로 안전함)
	if (AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>())
	{
		if (APawn* MyPawn = GetPawn())
		{
			if (GS)
			{
				GS->Multicast_SavePlayerLocation(PS->PS_UserIndex, MyPawn->GetActorTransform());
				UE_LOG(LogTemp, Warning, TEXT("[Exit] Saved Location for User %d"), PS->PS_UserIndex);
			}
		}
	}
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);


}

void ACuteAlienController::OnCancelUI()
{
	CancelRadialUI();

	if (HousingComp->PreviewTent)
	{
		HousingComp->PreviewTent->Destroy();
		HousingComp->PreviewTent = nullptr;
	}

	HousingComp->StopPreviewHousingItem();

	if (AMumulCharacter* MyChar = Cast<AMumulCharacter>(GetPawn()))
	{
		MyChar->SetFirstPersonView(false);
	}
}

void ACuteAlienController::OnToggleMouse()
{
	if (bShowMouseCursor)
	{
		SetIgnoreLookInput(false);
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
		return;
	}
	OnCancelUI();
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	SetIgnoreLookInput(true);
	SetShowMouseCursor(true);
	SetInputMode(InputMode);
}

void ACuteAlienController::OnClick(const FVector& TentLocation, const FRotator& TentRotation)
{
	// Place Tent
	if (HousingComp->PreviewTent)
	{
		// If Tent is Placeable
		if (HousingComp->PreviewTent->bIsPlaceable)
		{
			HousingComp->PreviewTent->Destroy();
			HousingComp->PreviewTent = nullptr;

			// Spawn or Move Tent
			HousingComp->Server_SpawnTent(FTransform(TentRotation, TentLocation));

			if (AMumulCharacter* MyChar = Cast<AMumulCharacter>(GetPawn()))
			{
				MyChar->SetFirstPersonView(false);
			}
		}
	}
}

void ACuteAlienController::ShowRadialUI()
{
	// Init UI
	OnCancelUI();

	// Lock Look Rotation
	SetIgnoreLookInput(true);
	// Hide Mouse Cursor
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

	//Show Radial UI
	RadialUI->SetVisibility(ESlateVisibility::Visible);
	RadialUI->PlaySlotSequence();
	bIsRadialVisible = true;
}

void ACuteAlienController::HideRadialUI()
{
	if (!RadialUI || RadialUI->GetVisibility() == ESlateVisibility::Hidden)
		return;

	SetIgnoreLookInput(false);
	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

	RadialUI->SetVisibility(ESlateVisibility::Hidden);
	bIsRadialVisible = false;

	// [추가] 선택된 슬롯 확인
	int32 SelectedIdx = RadialUI->GetCurrentSelectedIndex();
	UE_LOG(LogTemp, Log, TEXT("Selected Radial Slot: %d"), SelectedIdx);

	// 플레이어 가져오기
	ACuteAlienPlayer* CurPlayer = Cast<ACuteAlienPlayer>(GetPawn());
	if (!CurPlayer) return;

	CurPlayer->Server_PlayAlienDance(SelectedIdx);
}

void ACuteAlienController::CancelRadialUI()
{
	SetIgnoreLookInput(false);

	RadialUI->SetVisibility(ESlateVisibility::Hidden);
	bIsRadialVisible = false;
}

void ACuteAlienController::TryInitPlayerInfo()
{
	AMumulPlayerState* PS = GetPlayerState<AMumulPlayerState>();

	// PlayerState가 아직 없으면 다음 틱을 기다림
	if (!PS)
	{
		return;
	}

	// PlayerState가 유효하면 타이머 종료
	GetWorldTimerManager().ClearTimer(InitPlayerStateTimerHandle);

	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	if (GI)
	{
		// [체크] 로비 스킵 여부 확인 (테스트용)
		if (GI->PlayerUniqueID == 10)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Test] Detected Direct Level Start! Injecting Dummy Data..."));

			// GameState 확인 (안전장치)
			int32 CurrentPlayerCount = 0;
			if (AGameStateBase* CurrentGS = GetWorld()->GetGameState())
			{
				CurrentPlayerCount = CurrentGS->PlayerArray.Num();
			}

			GI->PlayerUniqueID = 9 + CurrentPlayerCount;
			GI->PlayerName = GI->PlayerName + FString::FromInt(GI->PlayerUniqueID);
			GI->CampID = 1;
			GI->PlayerType = (GI->PlayerUniqueID == 10) ? TEXT("운영진") : TEXT("학생");
			GI->PlayerTendency = 0;
			GI->bHasSurveyCompleted = true;
		}

		// [전송] 확정된 데이터를 서버로 1회 전송
		Server_InitPlayerInfo(
			GI->PlayerUniqueID,
			GI->PlayerName,
			GI->PlayerType,
			GI->PlayerTendency
		);

		UE_LOG(LogTemp, Log, TEXT("[Client] Sent Init Info: %s (ID: %d)"), *GI->PlayerName, GI->PlayerUniqueID);
	}
}


// void ACuteAlienController::Server_AddTeamChatList_Implementation(const FString& TeamID)
// {
// 	if (GS)
// 	{
// 		GS->AddTeamChatList(TeamID);
// 	}
// }
//
// void ACuteAlienController::OnServerCreateTeamChatResponse(bool bSuccess, FString Message)
// {
// 	if (bSuccess)
// 	{
// 		// 1. JSON 파싱 (Message에는 JSON 원본이 들어있음)
// 		FCreateTeamChatResponse CreateTeamChat;
//
// 		if (FJsonObjectConverter::JsonObjectStringToUStruct(Message, &CreateTeamChat, 0, 0))
// 		{
// 			// // JSON Parsing LOG
// 			// UE_LOG(LogTemp, Warning, TEXT("===== CreateTeamChat Response ====="));
// 			// UE_LOG(LogTemp, Warning, TEXT("groupId: %s"), *CreateTeamChat.groupId);
// 			// UE_LOG(LogTemp, Warning, TEXT("groupName: %s"), *CreateTeamChat.groupName);
// 			//
// 			// UE_LOG(LogTemp, Warning, TEXT("userIdList (%d명):"), CreateTeamChat.userIdList.Num());
// 			// for (int32 UserID : CreateTeamChat.userIdList)
// 			// {
// 			// 	UE_LOG(LogTemp, Warning, TEXT(" - userId: %d"), UserID);
// 			// }
// 			//
// 			// TArray<FTeamUser> TeamUserIDs;
// 			//
// 			// FTeamData NewTeamData;
// 			// NewTeamData.UniqueTeamID = CreateTeamChat.groupId;
// 			// NewTeamData.TeamName = CreateTeamChat.groupName;
// 			// NewTeamData.TeamMateList = CreateTeamChat.userIdList;
// 			//
// 			// for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
// 			// {
// 			// 	if (AMumulPlayerState* MPS = Cast<AMumulPlayerState>(PS))
// 			// 	{
// 			// 		MPS->PS_PlayerTeamList.Add(NewTeamData);
// 			// 		if (CreateTeamChat.userIdList.Contains(MPS->PS_UserIndex))
// 			// 		{
// 			// 			FTeamUser NewUser;
// 			// 			NewUser.UserId = MPS->PS_UserIndex;
// 			// 			NewUser.UserName = MPS->PS_RealName;
// 			// 			TeamUserIDs.Add(NewUser);
// 			// 		}
// 			// 	}
// 			// }
//
// 			if (IsLocalController())
// 			{
// 				Server_RequestTeamChatList();
// 				// Server_CreateGroupChatUI(CreateTeamChat.userIdList, CreateTeamChat.groupId, CreateTeamChat.groupName, TeamUserIDs);
// 			}
// 		}
// 		else
// 		{
// 			UE_LOG(LogTemp, Error, TEXT("CreateTeamChat 파싱 실패"));
// 		}
// 	}
// 	else
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("CreateTeamChat Response 실패 : %s"), *Message);
// 	}
// }


// void ACuteAlienController::Server_RequestTeamChatList_Implementation()
// {
// 	Multicast_RequestTeamChatList();
// }

// void ACuteAlienController::Multicast_RequestTeamChatList_Implementation()
// {
// 	// Get TeamChatList
// 	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
// 	{
// 		AMumulPlayerState* PS = PC->GetPlayerState<AMumulPlayerState>();
// 		HttpSystem->SendTeamChatListRequest(PS->PS_UserIndex);
// 	}
// }

// void ACuteAlienController::Server_CreateGroupChatUI_Implementation(const TArray<int32>& UserIDs, const FString& TeamID,
//                                                                    const FString& TeamName,
//                                                                    const TArray<FTeamUser>& TeamUserIDs)
// {
// 	if (IMGManager)
// 	{
// 		UTexture2D* TeamIconIMG = IMGManager->GetImageByTeamID(TeamID);
//
// 		// Add GroupChatUI for each Client
// 		for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
// 		{
// 			if (UserIDs.Contains(Cast<AMumulPlayerState>(PS)->PS_UserIndex))
// 			{
// 				if (ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController()))
// 				{
// 					PC->Client_CreateGroupChatUI(TeamID, TeamName, TeamUserIDs, TeamIconIMG);
// 				}
// 			}
// 		}
// 	}
// }

// void ACuteAlienController::Client_CreateGroupChatUI_Implementation(const FString& TeamID, const FString& TeamName,
//                                                                    const TArray<FTeamUser>& TeamUserIDs,
//                                                                    UTexture2D* IMG)
// {
// 	// Set Players in Group Icon
// 	// UGroupIconUI* GroupIconUI = CreateWidget<UGroupIconUI>(GetWorld(), GroupIconUIClass);
// 	// GroupChatUI->AddGroupIcon(GroupIconUI);
// 	// GroupIconUI->InitParentUI(GroupChatUI);
// 	// GroupIconUI->ChatBlockUI->SetTeamID(TeamID);
// 	// GroupIconUI->ChatBlockUI->SetTeamName(TeamName);
// 	// for (const FTeamUser& User : TeamUserIDs)
// 	// {
// 	// 	GroupIconUI->ChatBlockUI->AddTeamUser(User.UserId, User.UserName);
// 	// }
// 	// GroupIconUI->SetIconIMG(IMG);
// }


// void ACuteAlienController::Server_RequestChat_Implementation(const FString& TeamID, const TArray<int32>& UserIDs,
//                                                              const FString& CurrentTime, const int32& UserID,
//                                                              const FString& Name,
//                                                              const FString& Text)
// {
// 	// Add GroupChatUI for each Client
// 	for (APlayerState* PS : GetWorld()->GetGameState()->PlayerArray)
// 	{
// 		if (UserIDs.Contains(Cast<AMumulPlayerState>(PS)->PS_UserIndex))
// 		{
// 			if (ACuteAlienController* PC = Cast<ACuteAlienController>(PS->GetOwningController()))
// 			{
// 				PC->Client_SendChat(TeamID, CurrentTime, UserID, Name, Text);
// 			}
// 		}
// 	}
// }

// void ACuteAlienController::Client_SendChat_Implementation(const FString& TeamID, const FString& CurrentTime,
//                                                           const int32& UserID,
//                                                           const FString& Name, const FString& Text)
// {
// 	GroupChatUI->AddChat(TeamID, CurrentTime, UserID, Name, Text);
// }

void ACuteAlienController::Client_DisplayQuestion_Implementation(const int32& QuestionIdx, const FString& NewQuestion,
                                                                 const int32& QuestionTime)
{
	if (OXQuizUI)
	{
		OXQuizUI->SwitchQuizState(true);
		OXQuizUI->SetVisibility(ESlateVisibility::HitTestInvisible);
		OXQuizUI->SetQuizQuestion(QuestionIdx, NewQuestion);
		OXQuizUI->StartQuestionTimer(QuestionTime);
	}
}

void ACuteAlienController::Client_DisplayAnswer_Implementation(bool AnswerResult, bool NewAnswer,
                                                               const FString& NewCommentary, const int32& AnswerTime)
{
	bool CheckAnswer = false;
	if (AnswerResult == NewAnswer)
	{
		CheckAnswer = true;
	}

	OXQuizUI->SetQuizAnswer(CheckAnswer, NewAnswer, NewCommentary);
	OXQuizUI->StartAnswerTimer(AnswerTime);
}

void ACuteAlienController::Client_DisplayResult_Implementation(const int32& QuestionIdx, bool AnswerResult,
                                                               const FString& QuestionText,
                                                               bool AnswerText, const FString& CommentaryText)
{
	bool CheckAnswer = false;
	if (AnswerResult == AnswerText)
	{
		CheckAnswer = true;
	}

	OXQuizUI->SwitchQuizState(false);
	OXQuizUI->SetVisibility(ESlateVisibility::Visible);
	OXQuizUI->SetQuizResult(QuestionIdx, CheckAnswer, QuestionText, AnswerText, CommentaryText);
}
