// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/QuizQuestionUI.h"

#include "Components/TextBlock.h"


void UQuizQuestionUI::SetQuestionText(const int32& QuestionIdx, const FString& Text)
{
	
	QuestionText->SetText(FText::FromString(FString::Printf(TEXT("Q%d. %s"), QuestionIdx + 1, *Text)));
}
