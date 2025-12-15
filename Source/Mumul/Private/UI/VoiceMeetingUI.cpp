#include "UI/VoiceMeetingUI.h"
#include "Player/CuteAlienController.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Components/WidgetSwitcher.h"
#include "Player/MumulPlayerState.h"
#include "Player/Component/PlayerMeetingManagerComponent.h"
#include "UI/BaseUI/BaseButton.h"
#include "UI/BaseUI/BaseExitButton.h"
#include "UI/BaseUI/BaseTextBox.h"

void UVoiceMeetingUI::NativeConstruct()
{
    Super::NativeConstruct();
    
    if(MeetingStartBtn) MeetingStartBtn->BaseButton->OnClicked.AddDynamic(this, &UVoiceMeetingUI::OnClickStartMeeting);
    if(MeetingCancelBtn) MeetingCancelBtn->BaseExitButton->OnClicked.AddDynamic(this, &UVoiceMeetingUI::OnClickMeetingCancel);
    if(ConfirmEndBtn) ConfirmEndBtn->OnClicked.AddDynamic(this, &UVoiceMeetingUI::OnClickConfirmEnd);
    if(CancelEndBtn) CancelEndBtn->BaseButton->OnClicked.AddDynamic(this, &UVoiceMeetingUI::OnClickReturnMeeting);

    // 초기화
    if(MeetingWidgetSwitcher) MeetingWidgetSwitcher->SetActiveWidgetIndex(0);
}

void UVoiceMeetingUI::InitMeetingUI(bool bIsHost)
{
    // 방장이면 입력창 보임, 아니면 숨김
    if (MeetingWidgetSwitcher) MeetingWidgetSwitcher->SetActiveWidgetIndex(0);
    
    if (!bIsHost && MeetingStartBtn)
    {
        MeetingStartBtn->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UVoiceMeetingUI::SetMeetingState(bool bIsActive)
{
    // 회의 중이면 UI 숨김 (종료 버튼은 별도로 띄우거나, 이 위젯을 종료 팝업용으로만 씀)
    // 여기서는 "회의 시작됨 -> UI 숨김" 처리
    if (bIsActive)
    {
        SetVisibility(ESlateVisibility::Hidden);
        
        // // 입력 모드 복귀
        // if (ACuteAlienController* PC = GetMyController())
        // {
        //     PC->SetShowMouseCursor(false);
        //     PC->SetInputMode(FInputModeGameOnly());
        // }
    }
}

// [시작 요청]
void UVoiceMeetingUI::OnClickStartMeeting()
{
    ACuteAlienController* PC = GetMyController();
    if (PC)
    {
        AMumulPlayerState* PS = PC->GetPlayerState<AMumulPlayerState>();
        if (PS && !PS->bIsNearByCampFire)
        {
            // 예: 경고 메시지 출력 (화면 중앙 텍스트 등)
            UE_LOG(LogTemp, Warning, TEXT("모닥불 근처에서만 회의를 시작할 수 있습니다."));
            return;
        }

        FString Title = MeetingTitleText->BaseTextBox->GetText().ToString();
        if (Title.IsEmpty()) return;

        PC->MeetingComp->RequestStartMeetingRecording(Title, MeetingAgendaText->BaseTextBox->GetText().ToString(), MeetingDescText->BaseTextBox->GetText().ToString());
    }
}

// [종료 확인 - 예]
void UVoiceMeetingUI::OnClickConfirmEnd()
{
    ACuteAlienController* PC = GetMyController();
    if (PC)
    {
        // 컨트롤러에게 종료 요청 (RPC)
        PC->MeetingComp->RequestStopMeetingRecording();
    }
    SetVisibility(ESlateVisibility::Hidden);
}

// [종료 확인 - 아니오]
void UVoiceMeetingUI::OnClickReturnMeeting()
{
    SetVisibility(ESlateVisibility::Hidden);
    
    // // 입력 모드 게임으로 복귀
    // if (ACuteAlienController* PC = GetMyController())
    // {
    //     FInputModeGameOnly InputMode;
    //     PC->SetInputMode(InputMode);
    //     PC->SetShowMouseCursor(false);
    // }
}

ACuteAlienController* UVoiceMeetingUI::GetMyController()
{
    return Cast<ACuteAlienController>(GetOwningPlayer());
}

void UVoiceMeetingUI::OnClickMeetingCancel()
{
    OnClickReturnMeeting();
}
