#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VoiceMeetingUI.generated.h"

UCLASS()
class MUMUL_API UVoiceMeetingUI : public UUserWidget
{
	GENERATED_BODY()
    
protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher* MeetingWidgetSwitcher;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* TeamNameText;
	
	// [시작 화면]
	UPROPERTY(meta=(BindWidget))
	class UEditableTextBox* MeetingTitleText; // EditableText -> EditableTextBox 권장
	UPROPERTY(meta=(BindWidget))
	class UEditableTextBox* MeetingAgendaText;
	UPROPERTY(meta=(BindWidget))
	class UMultiLineEditableText* MeetingDescText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> MeetingStartBtn;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> MeetingCancelBtn;
    
	// [종료 화면]
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> ConfirmEndBtn; // "예"
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UButton> CancelEndBtn;  // "아니오"

	// 함수들
	void InitMeetingUI(bool bIsHost); // UI 초기화
	void SetMeetingState(bool bIsActive); // 화면 전환

private:
	class ACuteAlienController* GetMyController();

	UFUNCTION()
	void OnClickMeetingCancel();
	
	UFUNCTION()
	void OnClickStartMeeting();

	UFUNCTION()
	void OnClickConfirmEnd();

	UFUNCTION()
	void OnClickReturnMeeting();
};