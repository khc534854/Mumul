// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/FeedbackUI.h"

#include "Base/MumulGameInstance.h"
#include "Components/Button.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Network/HttpNetworkSubsystem.h"
#include "UI/BaseUI/BaseButton.h"
#include "UI/BaseUI/BaseText.h"
#include "UI/BaseUI/BaseTextBox.h"

void UFeedbackUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmEndBtn)
	{
		ConfirmEndBtn->OnClicked.AddDynamic(this, &UFeedbackUI::OnClickedConfirmEndBtn);
	}
	if (CancelEndBtn && CancelEndBtn->BaseButton)
	{
		CancelEndBtn->BaseButton->OnClicked.AddDynamic(this, &UFeedbackUI::OnClickedCancelEndBtn);
	}
	if (FeedbackExitBtn && FeedbackExitBtn->BaseButton)
	{
		FeedbackExitBtn->BaseButton->OnClicked.AddDynamic(this, &UFeedbackUI::OnClickedFeedbackExitBtn);
	}
}

void UFeedbackUI::OnClickedConfirmEndBtn()
{
	if (IsAnimationPlaying(Feedback_SlideAnim))
		return;
	
	FString ContentStr;
	if (FeedbackText)
	{
		// BaseTextBox 내부의 EditableTextBox에서 텍스트를 가져온다고 가정
		// 예: ContentStr = FeedbackText->GetText().ToString(); 
		// 만약 BaseTextBox가 UEditableTextBox를 상속받았다면:
		ContentStr = FeedbackText->BaseTextBox->GetText().ToString();
	}

	if (ContentStr.IsEmpty())
	{
		// 내용이 없으면 리턴 (추후 알림 메시지 추가 가능)
		return;
	}

	// 2. HTTP 요청 보내기
	UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance());
	if (GI)
	{
		UHttpNetworkSubsystem* Http = GI->GetSubsystem<UHttpNetworkSubsystem>();
		if (Http)
		{
			// 응답 델리게이트 바인딩 (이미 바인딩되어 있다면 Remove 후 Add)
			Http->OnFeedbackResponse.RemoveDynamic(this, &UFeedbackUI::OnFeedbackResponseReceived);
			Http->OnFeedbackResponse.AddDynamic(this, &UFeedbackUI::OnFeedbackResponseReceived);

			// 요청 전송 (PlayerID는 GI에서 가져옴)
			Http->SendFeedbackRequest(GI->PlayerUniqueID, ContentStr);

			// (선택) 버튼 비활성화 등 중복 전송 방지 처리
		}
	}
}

void UFeedbackUI::OnClickedCancelEndBtn()
{
	if (IsAnimationPlaying(Feedback_SlideAnim))
		return;

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	PlayAnimation(Feedback_SlideAnim, 0, 1, EUMGSequencePlayMode::Reverse);

	// 입력 모드 복구 (게임 모드로)
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetIgnoreLookInput(false);
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void UFeedbackUI::OnClickedFeedbackExitBtn()
{
	if (IsAnimationPlaying(Feedback_SlideAnim))
		return;

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	PlayAnimation(FeedbackResult_SlideAnim, 0, 1, EUMGSequencePlayMode::Reverse);

	// 입력 모드 복구 (게임 모드로)
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetIgnoreLookInput(false);
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void UFeedbackUI::OnFeedbackResponseReceived(bool bSuccess, FString Message)
{
	if (FeedbackWidgetSwitcher)
	{
		// 결과 화면(Index 1)으로 전환
		FeedbackWidgetSwitcher->SetActiveWidgetIndex(1);
		PlayAnimation(FeedbackResult_SlideAnim);
	}

	if (FeedbackResultText)
	{
		// 성공/실패 메시지 표시
		FString ResultMsg = bSuccess ? TEXT("피드백이 성공적으로 전송되었습니다.") : Message;
		FeedbackResultText->BaseText->SetText(FText::FromString(ResultMsg));
	}

	// 델리게이트 해제
	if (UMumulGameInstance* GI = Cast<UMumulGameInstance>(GetGameInstance()))
	{
		if (UHttpNetworkSubsystem* Http = GI->GetSubsystem<UHttpNetworkSubsystem>())
		{
			Http->OnFeedbackResponse.RemoveDynamic(this, &UFeedbackUI::OnFeedbackResponseReceived);
		}
	}
}

void UFeedbackUI::OpenFeedbackUI()
{
	if (IsAnimationPlaying(Feedback_SlideAnim))
		return;
	
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 첫 번째 화면(입력창)으로 전환
	if (FeedbackWidgetSwitcher)
	{
		FeedbackWidgetSwitcher->SetActiveWidgetIndex(0);
		PlayAnimation(Feedback_SlideAnim);
	}

	if (FeedbackText)
		FeedbackText->BaseTextBox->SetText(FText::GetEmpty());
}
