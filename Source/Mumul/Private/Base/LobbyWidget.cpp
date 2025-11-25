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
    if (gi)
       UE_LOG(LogTemp, Display, TEXT("%s"), *gi->GetSteamNickname());

    // --- [추가] 계정 정보 하드코딩 초기화 ---
    AccountMap.Add(TEXT("admin"), TEXT("1234")); // 관리자
    AccountMap.Add(TEXT("user1"), TEXT("1234")); // 일반 유저
    AccountMap.Add(TEXT("guest"), TEXT("1234")); // 게스트
    
    // --- [추가] 로그인 버튼 바인딩 ---
    if (btn_Login)
    {
        btn_Login->OnClicked.AddDynamic(this, &ULobbyWidget::OnClickLogin);
    }
    
    // 기존 버튼 바인딩
    btn_goCreate->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickGoCreate);
    btn_goFind->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickGoFind);
    btn_Create->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickCreate);
    editSessionName->OnTextChanged.AddDynamic(this,&ULobbyWidget::OnValudeChangedSessionName);
    sliderPlayerCount->OnValueChanged.AddDynamic(this,&ULobbyWidget::OnValudeChangedPlayerCount);
    btn_find->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickFind);
    gi->OnFindSessionsCompleteEvent.AddDynamic(this, &ULobbyWidget::RefreshSessionList);
    btn_BackFromFind->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickBack);
    btn_BackFromCreate->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickBack);

    // 시작 시 로그인 화면(0번) 보여주기
    if (WidgetSwitcher)
    {
        WidgetSwitcher->SetActiveWidgetIndex(0);
    }
}

// --- [추가] 로그인 처리 함수 ---
void ULobbyWidget::OnClickLogin()
{
    FString InputId = editLoginId->GetText().ToString();
    FString InputPw = editLoginPw->GetText().ToString();

    // 1. 아이디 존재 여부 및 비밀번호 일치 확인
    if (AccountMap.Contains(InputId) && AccountMap[InputId] == InputPw)
    {
        // 로그인 성공 처리
        textLoginMsg->SetText(FText::FromString(TEXT("로그인 성공!")));
        textLoginMsg->SetColorAndOpacity(FLinearColor::Green);
        Cast<UMumulGameInstance>(GetGameInstance())->MyLoginID = InputId;

        // 2. admin 여부에 따라 생성 버튼 가리기
        if (InputId == TEXT("admin"))
        {
            btn_goCreate->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            btn_goCreate->SetVisibility(ESlateVisibility::Collapsed); // 아예 안 보이게 처리
        }

        // 3. 메인 메뉴(Index 1)로 이동
        WidgetSwitcher->SetActiveWidgetIndex(1);
    }
    else
    {
        // 로그인 실패 처리
        textLoginMsg->SetText(FText::FromString(TEXT("아이디 또는 비밀번호가 틀렸습니다.")));
        textLoginMsg->SetColorAndOpacity(FLinearColor::Red);
    }
}

void ULobbyWidget::OnClickGoCreate()
{
    // [수정] 인덱스 번호 변경 (2번이 생성 화면이라고 가정)
    WidgetSwitcher->SetActiveWidgetIndex(2);
}

void ULobbyWidget::OnClickGoFind()
{
    // [수정] 인덱스 번호 변경 (3번이 찾기 화면이라고 가정)
    WidgetSwitcher->SetActiveWidgetIndex(3);
    OnClickFind();
}

void ULobbyWidget::OnClickCreate()
{
    FString sessionName = editSessionName->GetText().ToString();
    // 최소 2명 이상 보장
    int32 playerCount = FMath::Max(2, (int32)sliderPlayerCount->GetValue());
    FString mapURL = FString("/Game/Khc/Maps/Island?listen");
    gi->CreateGameSession(sessionName, playerCount, false, mapURL);
}

void ULobbyWidget::OnValudeChangedSessionName(const FText& text)
{
    btn_Create->SetIsEnabled(text.ToString().Len()>0);
}

void ULobbyWidget::OnValudeChangedPlayerCount(float value)
{
    textPlayerCount->SetText(FText::AsNumber(value));
}

void ULobbyWidget::OnClickFind()
{
    scrollSessionList->ClearChildren();
    textFind->SetText(FText::FromString(TEXT("세션 조회중")));
    btn_find->SetIsEnabled(false);
    gi->FindGameSessions();
}

void ULobbyWidget::OnClickBack()
{
    // [수정] 뒤로 가기 시 로그인 화면(0)이 아니라 메인 메뉴(1)로 이동
    WidgetSwitcher->SetActiveWidgetIndex(1);
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
    
    TArray<FOnlineSessionSearchResult> Results = gi->GetSessionSearch()->SearchResults;
    for (int32 i = 0; i < Results.Num(); i++)
    {
       if (!Results[i].IsValid()) continue;

       FString SessionName = Results[i].Session.OwningUserName;
        
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