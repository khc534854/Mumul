// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/OXQuizUI.h"

#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "UI/BaseUI/BaseText.h"
#include "UI/OXQuiz/QuizAnswerUI.h"
#include "UI/OXQuiz/QuizQuestionUI.h"

void UOXQuizUI::SwitchQuizState(const bool& QuizOrResult)
{
	if (QuizOrResult)
	{
		OXQuizWS->SetActiveWidgetIndex(0);
		return;
	}
	OXQuizWS->SetActiveWidgetIndex(1);
}

void UOXQuizUI::SetQuizQuestion(const FString& NewQuiz)
{
	QuizSizeBox->ClearChildren();
	
	UQuizQuestionUI* QuizQuestionUI = CreateWidget<UQuizQuestionUI>(GetWorld(), QuizQuestionUIClass);
	QuizQuestionUI->SetQuestionText(NewQuiz);
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

void UOXQuizUI::SetTimerText(const FString& NewTime)
{
	QuizTimerText->BaseText->SetText(FText::FromString(NewTime));
}
