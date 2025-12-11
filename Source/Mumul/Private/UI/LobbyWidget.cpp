// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/LobbyWidget.h"

#include "Network/HttpNetworkSubsystem.h"
#include "Base/MumulGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "UI/SessionInfoWidget.h"
#include "Components/ScrollBox.h"
#include "Player/MumulPlayerState.h"
#include "Network/NetworkStructs.h"
#include "UI/BaseUI/BaseButton.h"
#include "UI/BaseUI/BaseText.h"

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
    //btn_goCreate->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickGoCreate);
    //btn_goFind->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickGoFind);
    btn_Create->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickCreate);
    editSessionName->OnTextChanged.AddDynamic(this,&ULobbyWidget::OnValudeChangedSessionName);
    sliderPlayerCount->OnValueChanged.AddDynamic(this,&ULobbyWidget::OnValudeChangedPlayerCount);
    btn_find->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickFind);
    gi->OnFindSessionsCompleteEvent.AddDynamic(this, &ULobbyWidget::RefreshSessionList);
    //btn_BackFromFind->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickBack);
    //btn_BackFromCreate->OnClicked.AddDynamic(this,&ULobbyWidget::OnClickBack);

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
            HttpSystem->OnSurveyResultResponse.AddDynamic(this, &ULobbyWidget::OnSurveyResultResponse);
        }
    }

    if (btn_SurveyYes && btn_SurveyYes->BaseButton)
    {
        // 0번 인덱스 선택지 (일반적으로 '예' 또는 긍정)
        btn_SurveyYes->BaseButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnSurveyYesClicked_Internal);
    }
    if (btn_SurveyNo && btn_SurveyNo->BaseButton)
    {
        // 1번 인덱스 선택지 (일반적으로 '아니오' 또는 부정)
        btn_SurveyNo->BaseButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnSurveyNoClicked_Internal);
    }

    if (btn_Enter)
    {
        btn_Enter->BaseButton->OnClicked.AddDynamic(this, &ULobbyWidget::OnClickEnterGame);
        btn_Enter->SetIsEnabled(false); // 처음에는 비활성화 (검색 전)
    }
    
    LoadSurveyData();
}

// --- [추가] 로그인 처리 함수 ---
void ULobbyWidget::OnClickLogin()
{
    FString InputId = editLoginId->GetText().ToString();
    FString InputPw = editLoginPw->GetText().ToString();
    
    PendingID = InputId; // ID 임시 저장

    // [수정] 테스트 계정 예외 처리 및 임시 데이터 주입
    if (InputId == TEXT("admin"))
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
                if (GI->bHasSurveyCompleted)
                {
                    if (LoginData.tendencyTypeCode == TEXT("analyst")) GI->PlayerTendency = 1;
                    else if (LoginData.tendencyTypeCode == TEXT("doer")) GI->PlayerTendency = 2;
                    else if (LoginData.tendencyTypeCode == TEXT("balancer")) GI->PlayerTendency = 3;
                    else if (LoginData.tendencyTypeCode == TEXT("supporter")) GI->PlayerTendency = 4;
                    else if (LoginData.tendencyTypeCode == TEXT("pillar")) GI->PlayerTendency = 5;
                    else GI->PlayerTendency = 0;
                }
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
                if (GI->bHasSurveyCompleted)
                {
                    UpdateTendencyResultImage(GI->PlayerTendency);
                    WidgetSwitcher->SetActiveWidgetIndex(3);
                }
                else
                {
                    WidgetSwitcher->SetActiveWidgetIndex(1);
                    UpdateSurveyUI();
                    // http 통신 
                }
                //btn_goCreate->SetVisibility(ESlateVisibility::Collapsed);
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

void ULobbyWidget::UpdateTendencyResultImage(int32 TendencyID)
{
    if (!TendencyResultImg) return;

    // TendencyID는 1부터 시작하므로 배열 인덱스(0부터 시작)로 변환 (-1)
    int32 ImageIndex = TendencyID - 1;

    if (TendencyImages.IsValidIndex(ImageIndex) && TendencyImages[ImageIndex])
    {
        TendencyResultImg->SetBrushFromTexture(TendencyImages[ImageIndex]);
        UE_LOG(LogTemp, Log, TEXT("[UI] Tendency Image Updated for ID: %d"), TendencyID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[UI] Invalid Tendency ID or Texture missing. ID: %d"), TendencyID);
    }
}

void ULobbyWidget::LoadSurveyData()
{
    FString JsonFilePath = FPaths::ProjectContentDir() / TEXT("Data/personal_survey.json");
    
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *JsonFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("[Survey] Failed to load JSON file at: %s"), *JsonFilePath);
        return;
    }

    if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &SurveyData, 0, 0))
    {
        UE_LOG(LogTemp, Log, TEXT("[Survey] Successfully loaded %d questions."), SurveyData.questions.Num());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Survey] Failed to parse JSON data."));
    }
}

void ULobbyWidget::UpdateSurveyUI()
{
    if (SurveyData.questions.Num() == 0 || CurrentQuestionIndex >= SurveyData.questions.Num())
    {
        // 설문 완료 또는 데이터 없음
        return;
    }

    const FSurveyQuestion& CurrentQuestion = SurveyData.questions[CurrentQuestionIndex];
    int32 TotalQuestions = SurveyData.questions.Num();

    // 1. 질문 번호 업데이트
    if (QuestionCountText && QuestionCountText->BaseText)
    {
        FString CountStr = FString::Printf(TEXT("%d / %d"), CurrentQuestionIndex + 1, TotalQuestions);
        QuestionCountText->BaseText->SetText(FText::FromString(CountStr));
    }

    // 2. 질문 내용 업데이트
    if (QuestionText && QuestionText->BaseText)
    {
        QuestionText->BaseText->SetText(FText::FromString(CurrentQuestion.question));
    }
}

void ULobbyWidget::OnSurveyYesClicked_Internal()
{
    OnSurveyChoiceClicked(0);
}

void ULobbyWidget::OnSurveyNoClicked_Internal()
{
    OnSurveyChoiceClicked(1);
}

void ULobbyWidget::OnSurveyChoiceClicked(int32 ChoiceIndex)
{
    if (CurrentQuestionIndex >= SurveyData.questions.Num())
    {
        // 이미 완료됨
        return;
    }

    // 1. 결과 저장
    // (ChoiceIndex는 0 또는 1, value는 3 또는 1이지만, result 배열에는 index 값(0 또는 1)을 저장하므로 ChoiceIndex 저장)
    SurveyResults.Add(ChoiceIndex); 

    // 2. 다음 질문으로 이동
    CurrentQuestionIndex++;

    if (CurrentQuestionIndex < SurveyData.questions.Num())
    {
        // 다음 질문 업데이트
        UpdateSurveyUI();
    }
    else
    {
        // 3. 모든 질문 완료 -> 결과 제출
        SendSurveyResult();
    }
}

void ULobbyWidget::SendSurveyResult()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UHttpNetworkSubsystem* HttpSystem = GI->GetSubsystem<UHttpNetworkSubsystem>())
        {
            UMumulGameInstance* MumulGI = Cast<UMumulGameInstance>(GI);
            if (!MumulGI) return;
            
            // [수정] 새로운 SendSurveyResultRequest 함수 호출
            HttpSystem->SendSurveyResultRequest(MumulGI->PlayerUniqueID, SurveyResults);
            
            UE_LOG(LogTemp, Warning, TEXT("[Survey] Sending final result for User %d. Total %d responses."), 
                   MumulGI->PlayerUniqueID, SurveyResults.Num());
        }
    }
}

void ULobbyWidget::OnSurveyResultResponse(bool bSuccess, FString Message)
{
    if (bSuccess)
    {
        FSurveyResultResponse ResultData;
        if (FJsonObjectConverter::JsonObjectStringToUStruct(Message, &ResultData, 0, 0))
        {
            // 1. GameInstance에 성향 타입 코드 저장 (로그인 시의 로직과 유사)
            UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
            if (GI)
            {
                GI->bHasSurveyCompleted = true;

                // 성향 타입 코드를 숫자로 변환하여 저장 (예시: "analyst" -> 1)
                if (ResultData.typeCode == TEXT("analyst")) GI->PlayerTendency = 1;
                else if (ResultData.typeCode == TEXT("doer")) GI->PlayerTendency = 2;
                else if (ResultData.typeCode == TEXT("balancer")) GI->PlayerTendency = 3;
                else if (ResultData.typeCode == TEXT("supporter")) GI->PlayerTendency = 4;
                else if (ResultData.typeCode == TEXT("pillar")) GI->PlayerTendency = 5;
                else GI->PlayerTendency = 0;

                UE_LOG(LogTemp, Log, TEXT("[Survey] Result Received: %s (Tendency ID: %d)"), *ResultData.typeCode, GI->PlayerTendency);
            }
            
            // 2. 메인 메뉴 화면으로 전환 (1번 화면)
            UpdateTendencyResultImage(GI->PlayerTendency);
            WidgetSwitcher->SetActiveWidgetIndex(1); 
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[Survey] Result submission failed: %s"), *Message);
        // 사용자에게 실패 메시지 표시 (textLoginMsg 재활용 가능)
    }
}

void ULobbyWidget::OnClickEnterGame()
{
    if (FirstSessionIndex != -1)
    {
        // GameInstance를 통해 첫 번째 세션(인덱스 0)으로 조인 요청
        gi->JoinGameSession(FirstSessionIndex);
        
        // 중복 클릭 방지
        if (btn_Enter) btn_Enter->SetIsEnabled(false);
        if (textFind) textFind->SetText(FText::FromString(TEXT("입장 중...")));
    }
    else
    {
        // 혹시 모를 예외 처리 (검색 다시 시도 등)
        gi->FindGameSessions();
    }
}

//
// void ULobbyWidget::OnClickGoCreate()
// {
//     // [수정] 인덱스 번호 변경 (2번이 생성 화면이라고 가정)
//     WidgetSwitcher->SetActiveWidgetIndex(3);
// }
//
// void ULobbyWidget::OnClickGoFind()
// {
//     // [수정] 인덱스 번호 변경 (3번이 찾기 화면이라고 가정)
//     WidgetSwitcher->SetActiveWidgetIndex(3);
//     OnClickFind();
// }

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

// void ULobbyWidget::OnClickBack()
// {
//     // [수정] 뒤로 가기 시 로그인 화면(0)이 아니라 메인 메뉴(1)로 이동
//     WidgetSwitcher->SetActiveWidgetIndex(1);
// }

void ULobbyWidget::RefreshSessionList(bool bWasSuccessful)
{
    // 기존 리스트 UI 초기화 (보이지 않더라도 데이터 정리는 필요)
    if (scrollSessionList)
    {
        scrollSessionList->ClearChildren();
    }

    if (!bWasSuccessful || !gi->GetSessionSearch().IsValid())
    {
        if (textFind) textFind->SetText(FText::FromString(TEXT("세션 조회 실패 또는 없음")));
       
        // 검색 실패 시 입장 불가
        FirstSessionIndex = -1;
        if (btn_Enter) btn_Enter->SetIsEnabled(false);
       
        if (btn_find) btn_find->SetIsEnabled(true);
        return;
    }

    TArray<FOnlineSessionSearchResult> Results = gi->GetSessionSearch()->SearchResults;
    
    if (Results.Num() > 0)
    {
        // [핵심] 첫 번째 세션이 존재함 -> 입장 가능 상태로 변경
        FirstSessionIndex = 0; 
        
        if (btn_Enter) 
        {
            btn_Enter->SetIsEnabled(true);
        }

        if (textFind) 
        {
            FString FoundMsg = FString::Printf(TEXT("입장 가능 (%s)"), *Results[0].Session.OwningUserName);
            textFind->SetText(FText::FromString(FoundMsg));
        }

        // (선택) 디버깅용으로 리스트에도 추가할 수 있음 (UI가 Visible이라면)
        /*
        for (int32 i = 0; i < Results.Num(); i++)
        {
           if (!Results[i].IsValid()) continue;
           USessionInfoWidget* item = CreateWidget<USessionInfoWidget>(GetWorld(), sessionInfoWidget);
           if(item) {
              item->SetSessionInfo(i, Results[i].Session.OwningUserName);
              scrollSessionList->AddChild(item);
           }
        }
        */
    }
    else
    {
        // 검색 결과 0개 -> 입장 불가
        FirstSessionIndex = -1;
        if (btn_Enter) btn_Enter->SetIsEnabled(false);
        if (textFind) textFind->SetText(FText::FromString(TEXT("진행 중인 게임이 없습니다.")));
    }
    
    if (btn_find) btn_find->SetIsEnabled(true);
}