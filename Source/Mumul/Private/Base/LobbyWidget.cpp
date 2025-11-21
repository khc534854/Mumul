// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/LobbyWidget.h"

#include "MumulGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Base/SessionInfoWidget.h"
#include "Components/ScrollBox.h"

void ULobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();
	GetWorld()->GetFirstPlayerController()->SetShowMouseCursor(true);
	gi = Cast<UMumulGameInstance>(GetGameInstance());
	
	//세션 생성 버튼 클릭 함수 등록
	btn_goCreate->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickGoCreate);
	btn_goFind->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickGoFind);
	//생성 버튼 클릭 함수 등록
	btn_Create->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickCreate);
	//세션 이름 변경 될때 호출되는 함수
	editSessionName->OnTextChanged.AddDynamic(this,&ULobbyWidget::OnValudeChangedSessionName);
	//인원수 변경될때 호출되는 함수등록
	sliderPlayerCount->OnValueChanged.AddDynamic(this,&ULobbyWidget::OnValudeChangedPlayerCount);

	btn_find->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickFind);

	//게임 인스턴스의 델리게이트 등록
	gi->OnFindSessionsCompleteEvent.AddDynamic(this, &ULobbyWidget::RefreshSessionList);
	
	btn_BackFromFind->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickBack);

	btn_BackFromCreate->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickBack);
	
	
	
}

void ULobbyWidget::OnClickGoCreate()
{
	//세션 생성 화면으로 이동
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void ULobbyWidget::OnClickGoFind()
{
	WidgetSwitcher->SetActiveWidgetIndex(2);
	OnClickFind();
}

void ULobbyWidget::OnClickCreate()
{
	FString sessionName = editSessionName->GetText().ToString();
	int32 playerCount = sliderPlayerCount->GetValue();
	FString mapURL = FString("Game/ThirdPerson/Maps/ThirdPersonMap?listen");
	gi->CreateGameSession(sessionName, playerCount, false, mapURL);
}

void ULobbyWidget::OnValudeChangedSessionName(const FText& text)
{
	//text.IsEmpty()
	btn_Create->SetIsEnabled(text.ToString().Len()>0);
	
}



void ULobbyWidget::OnValudeChangedPlayerCount(float value)
{
	//valude값을 textPlayerCount에 설정
	textPlayerCount->SetText(FText::AsNumber(value));
}

void ULobbyWidget::OnClickFind()
{
	scrollSessionList->ClearChildren();
	textFind->SetText(FText::FromString(TEXT("세션 조회중")));
	btn_find->SetIsEnabled(false);
	//세션 조회
	gi->FindGameSessions();
}

void ULobbyWidget::OnFindComplete(int32 idx, FString sessionName)
{
	if (idx == -1)
	{
		textFind->SetText(FText::FromString(TEXT("세션 조회중")));
		btn_find->SetIsEnabled(true);
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("찾은 IDX : %d"),idx);
		//sessionInfoWidget 만들자
		USessionInfoWidget* item = CreateWidget<USessionInfoWidget>(GetWorld(),sessionInfoWidget);
		//만들어진 ITEM을 scrollSessionList에 추가
		scrollSessionList->AddChild(item);
		//만들어진 item정보 설정
		item->SetSessionInfo(idx,sessionName);
	}
	
}

void ULobbyWidget::OnClickBack()
{
	WidgetSwitcher->SetActiveWidgetIndex(0);
}

void ULobbyWidget::RefreshSessionList(bool bWasSuccessful)
{
	if (!bWasSuccessful || !gi->GetSessionSearch().IsValid())
	{
		textFind->SetText(FText::FromString(TEXT("세션 조회 실패 또는 없음")));
		btn_find->SetIsEnabled(true);
		return;
	}

	scrollSessionList->ClearChildren();
    
	// 검색된 결과 루프
	TArray<FOnlineSessionSearchResult> Results = gi->GetSessionSearch()->SearchResults;
	for (int32 i = 0; i < Results.Num(); i++)
	{
		// 유효한 세션인지 확인
		if (!Results[i].IsValid()) continue;

		FString SessionName = Results[i].Session.OwningUserName; // 보통 방장 이름으로 표시
        
		// 아이템 생성 및 추가
		USessionInfoWidget* item = CreateWidget<USessionInfoWidget>(GetWorld(), sessionInfoWidget);
		if(item)
		{
			item->SetSessionInfo(i, SessionName);
			scrollSessionList->AddChild(item);
		}
	}
    
	textFind->SetText(FText::FromString(TEXT("조회 완료")));
	btn_find->SetIsEnabled(true);
}
