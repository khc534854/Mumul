// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LogoutUI.h"

#include "Base/MumulGameInstance.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Data/AudioManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Network/WebSocketSubsystem.h"
#include "Player/CuteAlienController.h"
#include "UI/BaseUI/BaseButton.h"

void ULogoutUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (LogOutYesBtn)
	{
		LogOutYesBtn->OnClicked.AddDynamic(this, &ULogoutUI::OnClickedLogoutYesBtn);
	}
	if (LogOutNoBtn && LogOutNoBtn->BaseButton)
	{
		LogOutNoBtn->BaseButton->OnClicked.AddDynamic(this, &ULogoutUI::OnClickedLogoutNoBtn);
	}

	if (NextBtn && NextBtn->BaseButton)
	{
		NextBtn->BaseButton->OnClicked.AddDynamic(this, &ULogoutUI::OnClickedNextBtn);
	}

	PC = Cast<ACuteAlienController>(GetOwningPlayer());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find PlayerController"))
	}
}

void ULogoutUI::OnClickedLogoutYesBtn()
{
	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	if (GI)
	{
		UWebSocketSubsystem* WS = GI->GetSubsystem<UWebSocketSubsystem>();
		if (WS)
		{
			WS->Close();
		}
		
		UHttpNetworkSubsystem* Http = GI->GetSubsystem<UHttpNetworkSubsystem>();
		if (Http)
		{
			// 응답 델리게이트 바인딩 (이미 바인딩되어 있다면 Remove 후 Add)
			Http->OnLogoutResponse.RemoveDynamic(this, &ULogoutUI::OnLogoutResponseReceived);
			Http->OnLogoutResponse.AddDynamic(this, &ULogoutUI::OnLogoutResponseReceived);

			LogOutYesBtn->SetIsEnabled(false);
			LogOutNoBtn->SetIsEnabled(false);

			Http->SendLogoutRequest(GI->PlayerUniqueID);
		}
	}
}

void ULogoutUI::OnClickedLogoutNoBtn()
{
	if (IsAnimationPlaying(LogOut_SlideAnim))
		return;
	
	PlayAnimation(LogOut_SlideAnim, 0, 1, EUMGSequencePlayMode::Reverse);
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	PC->AudioManager->PlayPopDownSound();
	
	// 입력 모드 복구 (게임 모드로)
	if (PC)
	{
		PC->SetIgnoreLookInput(false);
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void ULogoutUI::OnLogoutResponseReceived(bool bSuccess, FString Message)
{
	if (UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance()))
	{
		if (UHttpNetworkSubsystem* Http = GI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			Http->OnLogoutResponse.RemoveDynamic(this, &ULogoutUI::OnLogoutResponseReceived);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[Logout] %s : %s"), bSuccess ? TEXT("Success" : TEXT("Failed")), *Message);
	
	PC->SaveAndExit();
}

void ULogoutUI::OpenHelpPopup()
{
	CurImageIdx = 0;
    
	// [신규] 첫 번째 이미지 설정
	if (HelpImages.IsValidIndex(0) && HelpImg)
	{
		HelpImg->SetBrushFromTexture(HelpImages[0]);
	}
}

void ULogoutUI::OnClickedNextBtn()
{
	if (IsAnimationPlaying(Help_SlideAnim))
		return;
	
	CurImageIdx++;
	
	if (!HelpImages.IsValidIndex(CurImageIdx))
	{
		// 도움말이 끝나면 창 닫기 (또는 로그아웃 페이지로 돌아가기)
		PlayAnimation(Help_SlideAnim, 0, 1, EUMGSequencePlayMode::Reverse);
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		PC->AudioManager->PlayPopDownSound();
	
		// 입력 모드 복구 (게임 모드로)
		if (PC)
		{
			PC->SetIgnoreLookInput(false);
			PC->SetShowMouseCursor(false);
			PC->SetInputMode(FInputModeGameOnly());
		}
		return;
	}
    
	if (HelpImg)
	{
		HelpImg->SetBrushFromTexture(HelpImages[CurImageIdx]);
	}
}
