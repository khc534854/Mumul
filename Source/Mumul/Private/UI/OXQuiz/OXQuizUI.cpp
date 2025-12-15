// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/OXQuizUI.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Data/ObjectAndClassFinder.h"
#include "UI/OXQuiz/AnswerCommentaryUI.h"
#include "UI/OXQuiz/AskOXQuizUI.h"
#include "UI/OXQuiz/QuizAnswerUI.h"
#include "UI/OXQuiz/QuizQuestionUI.h"


void UOXQuizUI::NativeConstruct()
{
	Super::NativeConstruct();

	ConfirmBtn->OnPressed.AddDynamic(this, &UOXQuizUI::OnConfirmResult);

	if (TSubclassOf<UAskOXQuizUI> AskOXQuizUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<UAskOXQuizUI>(
		"WBP_AskOXQuiz"))
	{
		AskOXQuizUI = CreateWidget<UAskOXQuizUI>(this, AskOXQuizUIClass);
		if (AskOXQuizUI)
		{
			QuizConfirmBox->AddChild(AskOXQuizUI);
			AskOXQuizUI->InitParentUI(this);
			AskOXQuizUI->OnCancelClicked.AddUObject(this, &UOXQuizUI::OnCancelClicked);
		}
	}
}

void UOXQuizUI::OnCancelClicked()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UOXQuizUI::SetTimerText(const int32& NewTime)
{
	if (QuizTimerText)
	{
		QuizTimerText->SetText(FText::FromString(FString::Printf(TEXT("%d초"), NewTime)));
	}
}

void UOXQuizUI::UpdateTimer()
{
	RemainingTime--;

	if (RemainingTime <= 0)
	{
		SetTimerText(0);

		GetWorld()->GetTimerManager().ClearTimer(QuizRemainingTimeHandler);
		return;
	}

	SetTimerText(RemainingTime);
}

void UOXQuizUI::OnConfirmResult()
{
	this->SetVisibility(ESlateVisibility::Hidden);
}

void UOXQuizUI::SwitchQuizState(const bool& QuizOrResult)
{
	if (QuizOrResult)
	{
		OXQuizWS->SetActiveWidgetIndex(0);
		AnswerListVBox->ClearChildren();
		return;
	}
	StopAnimation(TimerAnimation);
	OXQuizWS->SetActiveWidgetIndex(1);
}

void UOXQuizUI::SetQuizQuestion(const int32& QuestionIdx, const FString& NewQuiz)
{
	QuizSizeBox->ClearChildren();

	UQuizQuestionUI* QuizQuestionUI = CreateWidget<UQuizQuestionUI>(GetWorld(), QuizQuestionUIClass);
	QuizQuestionUI->SetQuestionText(QuestionIdx, NewQuiz);
	QuizSizeBox->AddChild(QuizQuestionUI);
}

void UOXQuizUI::SetQuizAnswer(const bool& AnswerResult, const bool& NewAnswer, const FString& NewCommentary)
{
	QuizSizeBox->ClearChildren();

	UQuizAnswerUI* QuizAnswerUI = CreateWidget<UQuizAnswerUI>(GetWorld(), QuizAnswerUIClass);
	QuizAnswerUI->SetAnswerColor(AnswerResult);
	QuizAnswerUI->SetAnswerResult(AnswerResult);

	QuizAnswerUI->SetQuizAnswer(NewAnswer);

	QuizAnswerUI->SetAnswerCommentary(NewCommentary);

	QuizSizeBox->AddChild(QuizAnswerUI);
}

void UOXQuizUI::StartQuestionTimer(const int32& QuestionTime)
{
	PlayAnimation(TimerAnimation, 0.f, 0);
	TimerIMG->SetColorAndOpacity(FLinearColor::Red);
	QuizTimerText->SetColorAndOpacity(FLinearColor::Red);

	RemainingTime = QuestionTime;

	SetTimerText(RemainingTime);
	GetWorld()->GetTimerManager().ClearTimer(QuizRemainingTimeHandler);

	GetWorld()->GetTimerManager().SetTimer(
		QuizRemainingTimeHandler,
		this,
		&UOXQuizUI::UpdateTimer,
		1.0f,
		true
	);
}

void UOXQuizUI::StartAnswerTimer(const int32& AnswerTime)
{
	StopAnimation(TimerAnimation);
	TimerIMG->SetColorAndOpacity(FLinearColor::Gray);
	QuizTimerText->SetColorAndOpacity(FLinearColor::Gray);

	RemainingTime = AnswerTime;

	SetTimerText(RemainingTime);
	GetWorld()->GetTimerManager().ClearTimer(QuizRemainingTimeHandler);

	GetWorld()->GetTimerManager().SetTimer(
		QuizRemainingTimeHandler,
		this,
		&UOXQuizUI::UpdateTimer,
		1.0f,
		true
	);
}

void UOXQuizUI::SetQuizResult(const int32& QuestionIdx, const bool& AnswerResult, const FString& QuestionText,
                              const bool& AnswerText, const FString& CommentaryText)
{
	UAnswerCommentaryUI* AnswerCommentaryUI = CreateWidget<UAnswerCommentaryUI>(GetWorld(), AnswerCommentaryUIClass);
	AnswerCommentaryUI->SetCommentaryColor(AnswerResult);
	AnswerCommentaryUI->SetQuestion(QuestionIdx, QuestionText);
	AnswerCommentaryUI->SetAnswer(AnswerText);
	AnswerCommentaryUI->SetCommentary(CommentaryText);

	if (UVerticalBoxSlot* VSlot = AnswerListVBox->AddChildToVerticalBox(AnswerCommentaryUI))
	{
		VSlot->SetPadding(FMargin(0.f, 5.7f));
	}
}
