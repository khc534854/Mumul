// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LogoutUI.h"

#include "Base/MumulGameInstance.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Network/HttpNetworkSubsystem.h"
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
		UHttpNetworkSubsystem* Http = GI->GetSubsystem<UHttpNetworkSubsystem>();
		if (Http)
		{
			// 응답 델리게이트 바인딩 (이미 바인딩되어 있다면 Remove 후 Add)
			Http->OnLogoutResponse.RemoveDynamic(this, &ULogoutUI::OnLogoutResponseReceived);
			Http->OnLogoutResponse.AddDynamic(this, &ULogoutUI::OnLogoutResponseReceived);

			Http->SendLogoutRequest(GI->PlayerUniqueID);
            
			// (선택) 버튼 비활성화 등 중복 전송 방지 처리
		}
	}
}

void ULogoutUI::OnClickedLogoutNoBtn()
{
	SetVisibility(ESlateVisibility::Hidden);

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
	PC->SaveAndExit();
}
