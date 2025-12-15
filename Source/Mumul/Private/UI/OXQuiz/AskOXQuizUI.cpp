// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/AskOXQuizUI.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/CuteAlienController.h"
#include "UI/BaseUI/BaseButton.h"
#include "UI/BaseUI/BaseText.h"
#include "UI/OXQuiz/OXQuizUI.h"

void UAskOXQuizUI::NativeConstruct()
{
	Super::NativeConstruct();

	ConfirmBtn->OnPressed.AddDynamic(this, &UAskOXQuizUI::OnConfirmQuiz);
	CancelBtn->BaseButton->OnPressed.AddDynamic(this, &UAskOXQuizUI::OnCancelQuiz);
}

void UAskOXQuizUI::OnConfirmQuiz()
{
	if (PC && QuizTriggerActor)
	{
		PC->Server_RequestStartQuiz(QuizTriggerActor);
		OXQuizUI->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAskOXQuizUI::OnCancelQuiz()
{
	OnCancelClicked.Broadcast();
	PC->OnToggleMouse();
}

void UAskOXQuizUI::InitParentUI(UOXQuizUI* Parent)
{
	OXQuizUI = Parent;
}

void UAskOXQuizUI::SetAskQuizText(const FText& DifficultyText)
{
	const FText FormatText = FText::FromString(TEXT("{0} 학습퀴즈를 진행하시겠습니까?"));
	const FText TextResult = FText::Format(FormatText, DifficultyText);

	AskQuizText->BaseText->SetText(TextResult);
}

void UAskOXQuizUI::SetQuizTriggerActor(AOXQuizTriggerActor* OXQuizTriggerActor)
{
	QuizTriggerActor = OXQuizTriggerActor;
}

void UAskOXQuizUI::SetPlayerController(ACuteAlienController* PlayerController)
{
	PC = PlayerController;
}
