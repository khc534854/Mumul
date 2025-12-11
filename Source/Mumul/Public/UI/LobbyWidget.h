// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Network/NetworkStructs.h"
#include "LobbyWidget.generated.h"

UCLASS()
class MUMUL_API ULobbyWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    
    UPROPERTY()
    class UMumulGameInstance* gi;

    UPROPERTY(meta=(BindWidget))
    class UWidgetSwitcher* WidgetSwitcher;

    // --- [기존] 메인 메뉴 ---
    // UPROPERTY(meta=(BindWidget))
    // class UButton* btn_goCreate;
    //
    // UPROPERTY(meta=(BindWidget))
    // class UButton* btn_goFind;
    
    // --- [추가] 로그인 화면 UI ---
    UPROPERTY(meta=(BindWidget))
    class UEditableTextBox* editLoginId; // 아이디 입력

    UPROPERTY(meta=(BindWidget))
    class UEditableTextBox* editLoginPw; // 비번 입력

    UPROPERTY(meta=(BindWidget))
    class UButton* btn_Login; // 로그인 버튼

    UPROPERTY(meta=(BindWidget))
    class UTextBlock* textLoginMsg; // 로그인 결과 메시지 (실패 등)

    // --- [기존] 세션 관련 ---
    UPROPERTY(meta=(BindWidget))
    class UScrollBox* scrollSessionList;
    
    UPROPERTY(meta=(BindWidget))
    class UButton* btn_find;

    UPROPERTY(meta=(BindWidget))
    class UTextBlock* textFind;

    //UPROPERTY(meta=(BindWidget))
    //class UButton* btn_BackFromFind;

    //UPROPERTY(meta=(BindWidget))
    //class UButton* btn_BackFromCreate;

    UPROPERTY(EditAnywhere)
    TSubclassOf<class USessionInfoWidget> sessionInfoWidget;
    
    UPROPERTY(meta=(BindWidget))
    class UEditableTextBox* editSessionName;
    
    UPROPERTY(meta=(BindWidget))
    class USlider* sliderPlayerCount;
    
    UPROPERTY(meta=(BindWidget))
    class UTextBlock* textPlayerCount;
    
    UPROPERTY(meta=(BindWidget))
    class UButton* btn_Create;

public:
    //UFUNCTION()
    //void OnClickGoCreate();
    //UFUNCTION()
    //void OnClickGoFind();
    UFUNCTION()
    void OnClickCreate();
    UFUNCTION()
    void OnValudeChangedSessionName(const FText& text);
    UFUNCTION()
    void OnValudeChangedPlayerCount(float value);
    UFUNCTION()
    void OnClickFind();
    //UFUNCTION()
    //void OnClickBack();
    UFUNCTION()
    void RefreshSessionList(bool bWasSuccessful);

    // --- [추가] 로그인 로직 ---
    UFUNCTION()
    void OnClickLogin(); // 로그인 버튼 클릭 시

private:
    // 아이디, 비밀번호 저장소 (Key: ID, Value: PW)
    TMap<FString, FString> AccountMap;

    UFUNCTION()
    void OnServerLoginResponse(bool bSuccess, FString Message);

    FString PendingID;

protected:
    UPROPERTY(meta=(BindWidget))
    class UBaseText* QuestionCountText;

    UPROPERTY(meta=(BindWidget))
    class UBaseText* QuestionText;
    
    UPROPERTY(meta=(BindWidget))
    class UBaseButton* btn_SurveyYes;
    
    UPROPERTY(meta=(BindWidget))
    class UBaseButton* btn_SurveyNo;

    UPROPERTY(meta=(BindWidget))
    class UBaseButton* btn_Enter;

    UPROPERTY(meta=(BindWidget))
    class UImage* TendencyResultImg;

    UPROPERTY(EditDefaultsOnly, Category = "Survey")
    TArray<TObjectPtr<class UTexture2D>> TendencyImages;

    void UpdateTendencyResultImage(int32 TendencyID);

    UPROPERTY()
    FSurveyListResponse SurveyData;

    int32 CurrentQuestionIndex = 0;
    TArray<int32> SurveyResults;

    // 1. 설문조사 JSON 로드 및 파싱 (NativeConstruct에서 호출)
    void LoadSurveyData();

    // 2. 현재 질문을 UI에 표시
    void UpdateSurveyUI();

    UFUNCTION()
    void OnSurveyYesClicked_Internal();
    
    UFUNCTION()
    void OnSurveyNoClicked_Internal();
    
    // 3. 버튼 클릭 처리
    UFUNCTION()
    void OnSurveyChoiceClicked(int32 ChoiceIndex);
    
    // 4. 설문 완료 시 HTTP 요청 전송
    void SendSurveyResult();

    // 5. 설문 결과 HTTP 응답 처리
    UFUNCTION()
    void OnSurveyResultResponse(bool bSuccess, FString Message);

    UFUNCTION()
    void OnClickEnterGame();
    
    int32 FirstSessionIndex = -1;
};