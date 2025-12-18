// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI.h"

#include "EngineUtils.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/FCustomItemData.h"
#include "Data/FHousingItemData.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Object/Tent/TentActor.h"
#include "Player/VoiceChatComponent.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "Player/MumulPlayerState.h"
#include "Player/Component/PlayerHousingSystemComponent.h"
#include "Player/Component/PlayerMeetingManagerComponent.h"
#include "save/MapDataSaveGame.h"
#include "UI/CustomItemEntryUI.h"
#include "UI/GroupChatUI.h"
#include "UI/BaseUI/BaseButton.h"
#include "UI/BaseUI/BaseText.h"

void UPlayerUI::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateCurrentTime();
	FDateTime Now = FDateTime::Now();
	int32 RemainingSeconds = 60 - Now.GetSecond();
	GetWorld()->GetTimerManager().SetTimer(
		FirstMinuteTimer,
		this,
		&UPlayerUI::StartMinuteTimer,
		RemainingSeconds,
		false
	);

	TentBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnTentClicked);
	MicrophoneBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnMicClicked);

	GetWorld()->GetTimerManager().SetTimer(
		GroupChatCheckTimer,
		this,
		&UPlayerUI::CheckGroupChatUI,
		0.7f,
		true
	);

	PC = Cast<ACuteAlienController>(GetOwningPlayer());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("RadialUI Can't Find PlayerController"))
	}

	GetWorld()->GetTimerManager().SetTimer(
		MinimapBindTimer, 
		this, 
		&UPlayerUI::TryBindMinimap, 
		0.5f, 
		true // 반복 실행
	);
	
	if (UVoiceChatComponent* VoiceComp = GetVoiceComponent())
	{
		UpdateMicButtonState(VoiceComp->IsSpeaking());
		UpdateRecordButtonState(VoiceComp->IsRecording());

		VoiceComp->OnSpeakingStateChanged.AddDynamic(this, &UPlayerUI::UpdateMicButtonState);
		VoiceComp->OnRecordingStateChanged.AddDynamic(this, &UPlayerUI::UpdateRecordButtonState);
	}

	LogOutBtn->SetVisibility(ESlateVisibility::Hidden);

	// 델리게이트 바인딩
	ProfileBtn->OnHovered.AddDynamic(this, &UPlayerUI::OnProfileBtnHovered);
	ProfileBtn->OnUnhovered.AddDynamic(this, &UPlayerUI::OnProfileBtnUnhovered);

	LogOutBtn->OnHovered.AddDynamic(this, &UPlayerUI::OnLogOutBtnHovered);
	LogOutBtn->OnUnhovered.AddDynamic(this, &UPlayerUI::OnLogOutBtnUnhovered);
	LogOutBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnLogOutBtnClicked);

	PlayerCustomizeBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnCustomizeBoxClick);
	HousingBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnHousingBoxClick);
	HousingDeleteModeBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnDeleteButtonClicked);

	if (PlayerCustomizeBtn) OriginalButtonStyles.Add(PlayerCustomizeBtn, PlayerCustomizeBtn->GetStyle());
	if (HousingBtn) OriginalButtonStyles.Add(HousingBtn, HousingBtn->GetStyle());
	if (TentBtn) OriginalButtonStyles.Add(TentBtn, TentBtn->GetStyle());
	
	LoadAndGenerateItemList();
	LoadAndGenerateHousingItemList();
}

void UPlayerUI::OnLogOutBtnClicked()
{
	//PC->SaveAndExit();
	PC->OpenLogoutUI();
}

void UPlayerUI::TryBindMinimap()
{
	if (!Minimap) 
	{
		// 미니맵 위젯 자체가 없으면 타이머 중지
		GetWorld()->GetTimerManager().ClearTimer(MinimapBindTimer);
		return;
	}

	// 내 캐릭터 가져오기
	ACuteAlienPlayer* MyPawn = Cast<ACuteAlienPlayer>(GetOwningPlayerPawn());

	// 1. 캐릭터가 존재하고
	// 2. 캐릭터가 렌더 타겟을 생성을 완료했는지 확인
	if (MyPawn && MyPawn->MinimapRenderTarget)
	{
		UMaterialInstanceDynamic* DynMat = Minimap->GetDynamicMaterial();
		if (DynMat)
		{
			// 텍스처 연결
			DynMat->SetTextureParameterValue(FName("MinimapTexture"), MyPawn->MinimapRenderTarget);
            
			UE_LOG(LogTemp, Warning, TEXT("[UI] Minimap Linked Successfully! Stopping Timer."));
            
			// 성공했으므로 타이머 종료 (더 이상 확인 안 함)
			GetWorld()->GetTimerManager().ClearTimer(MinimapBindTimer);
		}
	}
	else
	{
		// 아직 준비 안 됨. 다음 타이머 틱(0.5초 뒤)에 다시 시도.
		// UE_LOG(LogTemp, Log, TEXT("[UI] Waiting for Minimap RenderTarget..."));
	}
}

void UPlayerUI::StartMinuteTimer()
{
	UpdateCurrentTime();
	
	GetWorld()->GetTimerManager().SetTimer(
		TimeUpdater,
		this,
		&UPlayerUI::UpdateCurrentTime,
		60.f,
		true
	);
}

void UPlayerUI::UpdateCurrentTime()
{
	FDateTime Now = FDateTime::Now();
	FString TimeString = Now.ToString(TEXT("%Y. %m. %d  %H:%M"));
	//FString TimeString = Now.ToString(TEXT("%H:%M"));
	if (CurrentTime)
	{
		CurrentTime->BaseText->SetText(FText::FromString(TimeString));
	}
}

UVoiceChatComponent* UPlayerUI::GetVoiceComponent() const
{
	if (!PC) return nullptr;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return nullptr;

	return Pawn->FindComponentByClass<UVoiceChatComponent>();
}

void UPlayerUI::CheckGroupChatUI()
{
	if (GroupChatUI && GroupChatUI->RecordBtn)
	{
		GroupChatUI->RecordBtn->OnClicked.AddDynamic(this, &UPlayerUI::OnRecordClicked);
		
		GetWorld()->GetTimerManager().ClearTimer(GroupChatCheckTimer);
	}
}

void UPlayerUI::InitGroupChatUI(UGroupChatUI* UI)
{
	GroupChatUI = UI;
	if (GroupChatUI)
	{
		GroupChatUI->InitPlayerUI(this);
	}
}

void UPlayerUI::MarkHousingItemAsPlaced(FName ItemID, bool bPlaced)
{
	if (TObjectPtr<UCustomItemEntryUI>* FoundWidget = HousingWidgetMap.Find(ItemID))
	{
		if (*FoundWidget)
		{
			(*FoundWidget)->SetPlacedState(bPlaced);
		}
	}
}

void UPlayerUI::CheckEquippedCustomItem()
{
	AMumulPlayerState* PS = GetOwningPlayerState<AMumulPlayerState>();
	if (!PS) return;

	FName EquippedID = PS->EquippedCustomID;

	// 모든 커스텀 아이템 위젯 순회
	for (const TPair<FName, TObjectPtr<UCustomItemEntryUI>>& Pair : ItemWidgetMap)
	{
		if (Pair.Value)
		{
			// 현재 장착된 아이템이면 체크 켜기, 아니면 끄기
			// SetItemCheckState는 내부적으로 UpdateVisualState를 호출하므로 텍스트/색상도 자동 갱신됨
			bool bIsEquipped = (Pair.Key == EquippedID);
			Pair.Value->SetItemCheckState(bIsEquipped);
		}
	}
}

void UPlayerUI::CheckPlacedHousingItems()
{
	// 1. 내 UserIndex 가져오기
	int32 MyUserIndex = -1;
	if (AMumulPlayerState* PS = GetOwningPlayerState<AMumulPlayerState>())
	{
		MyUserIndex = PS->PS_UserIndex;
	}

	// 2. 월드에서 내 텐트 찾기
	ATentActor* MyTent = nullptr;
	for (TActorIterator<ATentActor> It(GetWorld()); It; ++It)
	{
		ATentActor* Tent = *It;
		if (Tent && Tent->OwnerUserIndex == MyUserIndex)
		{
			MyTent = Tent;
			break;
		}
	}

	// 3. 텐트에 저장된 아이템 목록을 순회하며 UI 업데이트
	if (MyTent)
	{
		for (const FHousingSaveData& Data : MyTent->HousingItems)
		{
			// 위젯 맵에 해당 아이템이 있다면 '배치됨'으로 설정
			MarkHousingItemAsPlaced(Data.ItemID, true);
		}
	}
}

void UPlayerUI::CancelTent()
{
	ResetAllMenuButtons();
}

void UPlayerUI::SetButtonActiveState(UButton* TargetBtn, bool bIsActive)
{
	if (!TargetBtn || !OriginalButtonStyles.Contains(TargetBtn)) return;

	// 1. 원래 스타일 가져오기
	FButtonStyle NewStyle = OriginalButtonStyles[TargetBtn];

	// 2. 활성화(ON) 상태라면: Normal 이미지를 Hovered 이미지로 교체
	if (bIsActive)
	{
		NewStyle.Normal = NewStyle.Pressed;
	}
	// 3. 비활성화(OFF) 상태라면: 원래 스타일 그대로 사용 (위에서 이미 복사됨)

	// 4. 스타일 적용
	TargetBtn->SetStyle(NewStyle);
}

void UPlayerUI::ResetAllMenuButtons()
{
	SetButtonActiveState(PlayerCustomizeBtn, false);
	SetButtonActiveState(HousingBtn, false);
	SetButtonActiveState(TentBtn, false);
}

bool UPlayerUI::TryLockUI(float Duration)
{
	// 이미 바쁘면(애니메이션 중이거나 방금 눌렀으면) true 반환 -> 함수 실행 막음
	if (bIsUIBusy) return false;

	// 잠금 걸기
	bIsUIBusy = true;

	// 지정된 시간(Duration) 뒤에 자동으로 잠금 해제
	GetWorld()->GetTimerManager().SetTimer(
		DebounceTimerHandle,
		this,
		&UPlayerUI::UnlockUIInteraction,
		Duration,
		false
	);

	return true; // 조작 성공
}

void UPlayerUI::OnCustomizeBoxClick()
{
	if (!TryLockUI()) return;
	
	if (GroupChatUI)
	{
		GroupChatUI->CloseChatUI();
	}

	bIsOpenCustomizeUI = !bIsOpenCustomizeUI;

	if (bIsOpenCustomizeUI)
	{
		if (bIsOpenHousingUI)
		{
			PlayAnimation(HousingBoxAnim, 0, 1, EUMGSequencePlayMode::Reverse);
			bIsOpenHousingUI = false;
			ResetHousingSelection();
			if (PC && PC->HousingComp) PC->HousingComp->StopPreviewHousingItem();
		}
        
		PlayAnimation(CustomizeBoxAnim, 0, 1, EUMGSequencePlayMode::Forward);
		CheckEquippedCustomItem();
	}
	else
	{
		PlayAnimation(CustomizeBoxAnim, 0, 1, EUMGSequencePlayMode::Reverse);
		PC->OnToggleMouse();
	}

	SetButtonActiveState(PlayerCustomizeBtn, bIsOpenCustomizeUI); // 내 버튼 상태 반영
	SetButtonActiveState(HousingBtn, false); // 다른 버튼 끄기
	SetButtonActiveState(TentBtn, false);
}

void UPlayerUI::LoadAndGenerateItemList()
{
	if (!CustomItemDataTable || !ItemEntryUIClass || !PC) return;
    
	// 현재 Pawn(캐릭터) 가져오기
	ACuteAlienPlayer* MyPawn = Cast<ACuteAlienPlayer>(GetOwningPlayerPawn());
	if (!MyPawn) return; 

	PlayerCustomizeItemBox->ClearChildren();
	ItemWidgetMap.Empty();

	// 1. 데이터 테이블 순회
	FString ContextString;
	TArray<FName> RowNames = CustomItemDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FCustomItemData* ItemData = CustomItemDataTable->FindRow<FCustomItemData>(RowName, ContextString);
		if (!ItemData) continue;

		// 2. 위젯 생성
		UCustomItemEntryUI* ItemUI = CreateWidget<UCustomItemEntryUI>(this, ItemEntryUIClass);
		if (ItemUI)
		{
			// 3. UI 초기화 및 데이터 전달
			ItemUI->InitItem(
			 	ItemData->ItemID, 
			 	ItemData->ItemThumbnail.LoadSynchronous(), 
			 	ItemData->ItemName.ToString(), 
			 	MyPawn,
			 	EItemEntryType::Custom 
			);
            
			// 4. 이벤트 바인딩
			ItemUI->OnItemChecked.AddDynamic(this, &UPlayerUI::OnCustomItemEntryChecked);

			// 5. ScrollBox에 추가
			PlayerCustomizeItemBox->AddChild(ItemUI);
			ItemWidgetMap.Add(ItemData->ItemID, ItemUI);
		}
	}
}

void UPlayerUI::OnCustomItemEntryChecked(FName ItemID, bool bIsChecked)
{
	AMumulPlayerState* PS = GetOwningPlayerState<AMumulPlayerState>();
	if (!PS) return;

	// 현재 캐릭터 (Pawn) 가져오기
	ACuteAlienPlayer* Player = Cast<ACuteAlienPlayer>(GetOwningPlayerPawn());
	if (!Player) return;

	if (bIsChecked)
	{
		// 1. 서버에 장착 요청
		Player->Server_EquipCustom(ItemID);

		// 2. [단일 장착 로직] 다른 모든 항목 체크 해제
		for (const TPair<FName, TObjectPtr<UCustomItemEntryUI>>& Pair : ItemWidgetMap)
		{
			if (Pair.Key != ItemID)
			{
				if (Pair.Value->ItemCheckBox && Pair.Value->ItemCheckBox->IsChecked())
				{
					// 다른 위젯의 체크박스 상태를 강제로 변경
					Pair.Value->SetItemCheckState(false);
				}
			}
		}
	}
	else
	{
		// 3. 서버에 해제 요청 (현재 체크 해제된 항목이 장착된 항목일 때만)
		if (PS->EquippedCustomID == ItemID) 
		{
			Player->Server_EquipCustom(NAME_None); // NAME_None은 해제를 의미
		}
	}
}

void UPlayerUI::UpdateMicButtonState(bool bActive)
{
	if (bActive)
	{
		PlayAnimation(MicOn, 0, 0);
	}
	else
	{
		StopAnimation(MicOn);
	}
	ChangeMicStateImage();
}

void UPlayerUI::UpdateRecordButtonState(bool bActive)
{
}

void UPlayerUI::OnTentClicked()
{
	if (!TryLockUI(0.5f)) return;
	
	PC->HousingComp->ShowPreviewTent();
	CloseSidePanels();

	SetButtonActiveState(TentBtn, true);
    
	// 텐트 설치 시 다른 UI가 닫혀야 한다면 아래 코드 추가
	if (bIsOpenCustomizeUI) OnCustomizeBoxClick(); // 혹은 강제 닫기 로직
	if (bIsOpenHousingUI) OnHousingBoxClick();
    
	// 시각적 업데이트 (위에서 닫기 로직 돌면서 꺼졌을 수 있으니 다시 확실하게)
	SetButtonActiveState(PlayerCustomizeBtn, false);
	SetButtonActiveState(HousingBtn, false);
	SetButtonActiveState(TentBtn, true);
}

void UPlayerUI::OnDeleteButtonClicked()
{
	if (PC && PC->HousingComp)
	{
		ResetHousingSelection(); 
        
		PC->HousingComp->StartHousingDeleteMode();
	}
}

void UPlayerUI::OnHousingBoxClick()
{
	if (!TryLockUI()) return;
	
	if (GroupChatUI)
	{
		GroupChatUI->CloseChatUI();
	}

	bIsOpenHousingUI = !bIsOpenHousingUI;

	ResetHousingSelection();
	if (bIsOpenHousingUI)
	{
		if (bIsOpenCustomizeUI)
		{
			PlayAnimation(CustomizeBoxAnim, 0, 1, EUMGSequencePlayMode::Reverse);
			bIsOpenCustomizeUI = false;
		}
        
		PlayAnimation(HousingBoxAnim, 0, 1, EUMGSequencePlayMode::Forward);
		CheckPlacedHousingItems();
	}
	else
	{
		PlayAnimation(HousingBoxAnim, 0, 1, EUMGSequencePlayMode::Reverse);
		PC->OnToggleMouse();
	}
	
	SetButtonActiveState(HousingBtn, bIsOpenHousingUI);
	SetButtonActiveState(PlayerCustomizeBtn, false);
	SetButtonActiveState(TentBtn, false);
}

void UPlayerUI::LoadAndGenerateHousingItemList()
{
	if (!HousingItemDataTable || !HousingItemEntryUIClass || !PC) return;
    
	HousingItemBox->ClearChildren();
	HousingWidgetMap.Empty();

	// 현재 캐릭터 (Pawn) 가져오기
	ACuteAlienPlayer* MyPawn = Cast<ACuteAlienPlayer>(GetOwningPlayerPawn());
	if (!MyPawn) return; 
	
	// 1. 데이터 테이블 순회
	FString ContextString;
	TArray<FName> RowNames = HousingItemDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FHousingItemData* HousingItemData = HousingItemDataTable->FindRow<FHousingItemData>(RowName, ContextString);
		if (!HousingItemData) continue;

		// 2. 위젯 생성
		UCustomItemEntryUI* ItemUI = CreateWidget<UCustomItemEntryUI>(this, HousingItemEntryUIClass);
		if (ItemUI)
		{
			// 3. UI 초기화 및 데이터 전달
			ItemUI->InitItem(
				HousingItemData->ItemID, 
				HousingItemData->ItemThumbnail.LoadSynchronous(), 
				HousingItemData->ItemName.ToString(), 
				MyPawn,
				EItemEntryType::Housing
			);
            
			// 4. 이벤트 바인딩
			ItemUI->OnItemChecked.AddDynamic(this, &UPlayerUI::OnHousingItemEntryChecked);

			// 5. ScrollBox에 추가
			HousingItemBox->AddChild(ItemUI);
			HousingWidgetMap.Add(HousingItemData->ItemID, ItemUI);

		}
	}
}

void UPlayerUI::OnHousingItemEntryChecked(FName ItemID, bool bIsChecked)
{
	if (!PC) return;

	if (bIsChecked)
	{
		// 1. [단일 선택] 다른 모든 항목 체크 해제
		for (const TPair<FName, TObjectPtr<UCustomItemEntryUI>>& Pair : HousingWidgetMap)
		{
			if (Pair.Key != ItemID)
			{
				if (Pair.Value->ItemCheckBox && Pair.Value->ItemCheckBox->IsChecked())
				{
					// 이벤트 전파 없이 상태만 변경 (무한 루프 방지)
					Pair.Value->SetItemCheckState(false);
				}
			}
		}

		// 2. 컨트롤러에게 해당 아이템의 프리뷰 모드 시작 요청
		PC->HousingComp->ShowPreviewHousingItem(ItemID);
	}
	else
	{
		// 3. 체크 해제 시 하우징 모드 종료 (프리뷰 제거)
		// 만약 현재 프리뷰 중인 아이템과 동일하다면 취소
		PC->HousingComp->StopPreviewHousingItem();
	}

	CheckPlacedHousingItems();
}

void UPlayerUI::ResetHousingSelection()
{
	for (const TPair<FName, TObjectPtr<UCustomItemEntryUI>>& Pair : HousingWidgetMap)
	{
		if (Pair.Value && Pair.Value->ItemCheckBox)
		{
			Pair.Value->SetItemCheckState(false);
		}
	}
}

void UPlayerUI::OnMicClicked()
{
	if (!TryLockUI(0.5f)) return;
	
	if (UVoiceChatComponent* VoiceComp = GetVoiceComponent())
	{
		VoiceComp->ToggleSpeaking();
	}
}

void UPlayerUI::OnRecordClicked()
{
	if (!TryLockUI(0.5f)) return;
	
	if (UVoiceChatComponent* VoiceComp = GetVoiceComponent())
	{
		if (PC)
		{
			// 녹음 중이면 -> 종료 팝업 띄우기
			if (VoiceComp->IsRecording())
			{
				PC->MeetingComp->OpenEndMeetingPopup();
				return;
			}
			
			AMumulPlayerState* PS = PC->GetPlayerState<AMumulPlayerState>();
			if (PS)
			{
				if (!PS->bIsNearByCampFire)
				{
					if (GroupChatUI)
					{
						GroupChatUI->AddBotChat(TEXT("모닥불 근처에서만 회의를 시작할 수 있습니다."));
					}
					return;
				}
			}
			PC->MeetingComp->OpenMeetingSetupUI();
		}
	}
}

void UPlayerUI::SetProfileBtnIMG(UTexture2D* IMG)
{
	FSlateBrush NormalBrush;
	NormalBrush.SetResourceObject(IMG);
	NormalBrush.ImageSize = FVector2D(30.f);
	NormalBrush.TintColor = FSlateColor(FLinearColor(1.f, 1.f, 1.f, 1.f));

	FSlateBrush HoveredBrush = NormalBrush;
	HoveredBrush.TintColor = FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f, 1.f));

	FSlateBrush PressedBrush = NormalBrush;
	PressedBrush.TintColor = FSlateColor(FLinearColor(0.75f, 0.75f, 0.75f, 1.f));

	FButtonStyle Style;
	Style.Normal = NormalBrush;
	Style.Hovered = HoveredBrush;
	Style.Pressed = PressedBrush;

	ProfileBtn->SetStyle(Style);
}

void UPlayerUI::CloseSidePanels()
{
	// 1. 커스터마이징 UI가 열려있다면 닫기
	if (bIsOpenCustomizeUI)
	{
		bIsOpenCustomizeUI = false;
		PlayAnimation(CustomizeBoxAnim, 0, 1, EUMGSequencePlayMode::Reverse);
	}

	// 2. 하우징 UI가 열려있다면 닫기
	if (bIsOpenHousingUI)
	{
		bIsOpenHousingUI = false;
		PlayAnimation(HousingBoxAnim, 0, 1, EUMGSequencePlayMode::Reverse);
		ResetHousingSelection();
		// 하우징 프리뷰 종료 등 필요한 로직 수행
		if (PC && PC->HousingComp)
		{
			PC->HousingComp->StopPreviewHousingItem();
		}
	}

	ResetAllMenuButtons();
}

void UPlayerUI::OnProfileBtnHovered()
{
	bIsMainHovered = true;
	GetWorld()->GetTimerManager().ClearTimer(HideLogOutTimer);
	bIsTryingToHide = false;
	LogOutBtn->SetVisibility(ESlateVisibility::Visible);
}

void UPlayerUI::OnProfileBtnUnhovered()
{
	bIsMainHovered = false;
	TryHideLogOutBtn();
}

void UPlayerUI::OnLogOutBtnHovered()
{
	bIsSubHovered = true;
	GetWorld()->GetTimerManager().ClearTimer(HideLogOutTimer);
	bIsTryingToHide = false;
}

void UPlayerUI::OnLogOutBtnUnhovered()
{
	bIsSubHovered = false;
	TryHideLogOutBtn();
}

void UPlayerUI::TryHideLogOutBtn()
{
	if (bIsTryingToHide)
		return;

	bIsTryingToHide = true;
	
	GetWorld()->GetTimerManager().SetTimer(
		HideLogOutTimer,
		this,
		&UPlayerUI::HideLogOutBtn,
		0.05f,
		false
	);
}

void UPlayerUI::HideLogOutBtn()
{
	bIsTryingToHide = false;
	
	if (bIsMainHovered || bIsSubHovered)
		return;

	LogOutBtn->SetVisibility(ESlateVisibility::Hidden);
}
