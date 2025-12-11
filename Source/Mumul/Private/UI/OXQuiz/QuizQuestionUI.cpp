// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/QuizQuestionUI.h"

#include "Components/TextBlock.h"
#include "UI/BaseUI/BaseText.h"

void UQuizQuestionUI::SetQuestionText(const FString& Text)
{
	QuestionText->BaseText->SetText(FText::FromString(Text));
}
