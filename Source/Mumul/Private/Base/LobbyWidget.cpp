// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/LobbyWidget.h"

#include "HttpNetworkSubsystem.h"
#include "MumulGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Base/SessionInfoWidget.h"
#include "Components/ScrollBox.h"
#include "khc/Player/MumulPlayerState.h"
#include "khc/System/NetworkStructs.h"

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

    if (UGameInstance* GI = GetGameInstance())
    {
        if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
        {
            HttpSystem->OnLoginResponse.AddDynamic(this, &ULobbyWidget::OnServerLoginResponse);
        }
    }
}

// --- [추가] 로그인 처리 함수 ---
void ULobbyWidget::OnClickLogin()
{
    FString InputId = editLoginId->GetText().ToString();
    FString InputPw = editLoginPw->GetText().ToString();
    
    PendingID = InputId; // ID 임시 저장

    // [수정] 테스트 계정 예외 처리 및 임시 데이터 주입
    if (InputId == TEXT("admin") || InputId == TEXT("user1"))
    {
        if (AccountMap.Contains(InputId) && AccountMap[InputId] == InputPw)
        {
            // --- [신규] 테스트용 임시 데이터 설정 ---
            UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
            if (GI)
            {
                // 100번대 ID 부여 (admin=100, user1=101)
                GI->PlayerUniqueID = (InputId == TEXT("admin")) ? 10 : 11;
                GI->PlayerName = GI->PlayerName + FString::FromInt(GI->PlayerUniqueID); // 이름 + index
                GI->CampID = 1;           // 임시 캠프 ID
                GI->PlayerType = (InputId == TEXT("admin")) ? TEXT("운영진") : TEXT("학생");
                GI->PlayerTendency = 0;
                GI->bHasSurveyCompleted = true;

                UE_LOG(LogTemp, Warning, TEXT("[Test Login] Set Dummy Data for %s (ID: %d)"), *InputId, GI->PlayerUniqueID);
            }

            if (InputId == TEXT("admin"))
                WidgetSwitcher->SetActiveWidgetIndex(2); // 방 생성 화면으로
            else
                WidgetSwitcher->SetActiveWidgetIndex(1);
                
            // 성공 메시지만 띄우고 데이터 처리는 위에서 끝냄
            textLoginMsg->SetText(FText::FromString(TEXT("테스트 로그인 성공")));
            textLoginMsg->SetColorAndOpacity(FLinearColor::Green);
        }
        else
        {
            textLoginMsg->SetText(FText::FromString(TEXT("비밀번호 틀림")));
            textLoginMsg->SetColorAndOpacity(FLinearColor::Red);
        }
        return; 
    }

    // 2. 그 외 계정은 서버로 요청 전송
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
        {
            // 버튼 비활성화 (중복 클릭 방지)
            btn_Login->SetIsEnabled(false);
            textLoginMsg->SetText(FText::FromString(TEXT("서버 확인 중...")));
            
            HttpSystem->SendLoginRequest(InputId, InputPw);
        }
    }
}

void ULobbyWidget::OnServerLoginResponse(bool bSuccess, FString Message)
{
    btn_Login->SetIsEnabled(true);

    if (bSuccess)
    {
        // 1. JSON 파싱 (Message에는 JSON 원본이 들어있음)
        FLoginSuccessResponse LoginData;
        if (FJsonObjectConverter::JsonObjectStringToUStruct(Message, &LoginData, 0, 0))
        {
            // 2. UI 메시지 업데이트
            FString WelcomeMsg = FString::Printf(TEXT("%s님 환영합니다."), *LoginData.name);
            textLoginMsg->SetText(FText::FromString(WelcomeMsg));
            textLoginMsg->SetColorAndOpacity(FLinearColor::Green);

            // 3. GameInstance에 정보 저장
            UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
            if (GI)
            {
                GI->PlayerUniqueID = LoginData.userId; 
                GI->PlayerName = LoginData.name;     
                GI->CampID = LoginData.campId;
                GI->PlayerType = LoginData.userType;
                GI->bHasSurveyCompleted = LoginData.tendencyCompleted;
                // ... 필요한 정보 다 저장
            }

            // 5. 관리자 여부 확인 및 화면 이동
            // (UserType이나 Name으로 판단. 여기선 기존대로 입력한 ID로 판단하거나 서버 데이터 활용)
            if (PendingID == TEXT("admin") || LoginData.name == TEXT("관리자") || PendingID == TEXT("admin1") || GI->PlayerType == TEXT("운영진")) 
            {
                //btn_goCreate->SetVisibility(ESlateVisibility::Visible);
                WidgetSwitcher->SetActiveWidgetIndex(2);
            }
            else if (PendingID == TEXT("user1") || GI->PlayerType == TEXT("학생"))
            {
                //btn_goCreate->SetVisibility(ESlateVisibility::Collapsed);
                WidgetSwitcher->SetActiveWidgetIndex(1);
            }

            // 다음 화면으로
        }
        else
        {
            // JSON 파싱 실패 시
            textLoginMsg->SetText(FText::FromString(TEXT("데이터 처리 오류")));
            textLoginMsg->SetColorAndOpacity(FLinearColor::Red);
        }
    }
    else
    {
        // 실패 시 Message는 에러 메시지 텍스트임
        textLoginMsg->SetText(FText::FromString(Message));
        textLoginMsg->SetColorAndOpacity(FLinearColor::Red);
    }
}


void ULobbyWidget::OnClickGoCreate()
{
    // [수정] 인덱스 번호 변경 (2번이 생성 화면이라고 가정)
    WidgetSwitcher->SetActiveWidgetIndex(3);
}

void ULobbyWidget::OnClickGoFind()
{
    // [수정] 인덱스 번호 변경 (3번이 찾기 화면이라고 가정)
    WidgetSwitcher->SetActiveWidgetIndex(3);
    OnClickFind();
}

void ULobbyWidget::OnClickCreate()
{
    //FString sessionName = editSessionName->GetText().ToString();
    FString sessionName = FString("Mumul");
    FString mapURL = FString("/Game/Khc/Maps/Island?listen");
    // 최소 2명 이상 보장
    //int32 playerCount = FMath::Max(2, (int32)sliderPlayerCount->GetValue());
    //gi->CreateGameSession(sessionName, playerCount, false, mapURL);
    gi->CreateGameSession(sessionName, 20, false, mapURL);
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