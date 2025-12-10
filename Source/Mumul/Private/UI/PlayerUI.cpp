// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/FCustomItemData.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Player/VoiceChatComponent.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "Player/MumulPlayerState.h"
#include "UI/CustomItemEntryUI.h"
#include "UI/GroupChatUI.h"
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
	//
	// if (Minimap)
	// {
	// 	// 내 캐릭터(Pawn) 가져오기
	// 	ACuteAlienPlayer* MyPawn = Cast<ACuteAlienPlayer>(GetOwningPlayerPawn());
 //        
	// 	// 캐릭터가 있고, 캐릭터 안에 렌더 타겟이 생성되어 있다면
	// 	if (MyPawn && MyPawn->MinimapRenderTarget)
	// 	{
	// 		// 1. 이미지 위젯의 머티리얼을 동적(Dynamic)으로 변환하여 가져옴
	// 		UMaterialInstanceDynamic* DynMat = Minimap->GetDynamicMaterial();
 //            
	// 		if (DynMat)
	// 		{
	// 			// 2. 머티리얼의 텍스처 파라미터에 렌더 타겟을 할당
	// 			// 주의: "MinimapTexture" 부분은 실제 머티리얼의 파라미터 이름과 똑같아야 합니다!
	// 			DynMat->SetTextureParameterValue(FName("MinimapTexture"), MyPawn->MinimapRenderTarget);
 //                
	// 			UE_LOG(LogTemp, Log, TEXT("[UI] Minimap RenderTarget Assigned Successfully!"));
	// 		}
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogTemp, Warning, TEXT("[UI] Failed to get MinimapRenderTarget from Pawn."));
	// 	}
	// }
	
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
	LoadAndGenerateItemList();
}

void UPlayerUI::OnLogOutBtnClicked()
{
	PC->SaveAndExit();	
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
	FString TimeString = Now.ToString(TEXT("%H:%M"));
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
}

void UPlayerUI::OnCustomizeBoxClick()
{
	bIsOpenCustomizeUI = !bIsOpenCustomizeUI;

	if (bIsOpenCustomizeUI)
	{
		PlayAnimation(CustomizeBoxAnim, 0, 1, EUMGSequencePlayMode::Forward);
	}
	else
	{
		PlayAnimation(CustomizeBoxAnim, 0, 1, EUMGSequencePlayMode::Reverse);
	}
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
				MyPawn // 캐릭터 전달
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
					Pair.Value->ItemCheckBox->SetIsChecked(false);
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
	PC->ShowPreviewTent();
}

void UPlayerUI::OnMicClicked()
{
	if (UVoiceChatComponent* VoiceComp = GetVoiceComponent())
	{
		VoiceComp->ToggleSpeaking();
	}
}

void UPlayerUI::OnRecordClicked()
{
	if (UVoiceChatComponent* VoiceComp = GetVoiceComponent())
	{
		if (PC)
		{
			// 녹음 중이면 -> 종료 팝업 띄우기
			if (VoiceComp->IsRecording())
			{
				PC->OpenEndMeetingPopup();
			}
			// 녹음 중 아니면 -> 시작 설정 UI 띄우기
			else
			{
				PC->OpenMeetingSetupUI();
			}
		}
	}
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
