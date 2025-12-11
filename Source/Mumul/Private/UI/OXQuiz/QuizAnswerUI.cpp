// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/QuizAnswerUI.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "UI/BaseUI/BaseText.h"



void UQuizAnswerUI::SetAnswerColor(bool TrueGreenOrFalseRed)
{
	if (TrueGreenOrFalseRed)
	{
		AnswerBorder->SetBrushColor(FLinearColor(0.025187f, 0.48515f, 0.116971f));
		return;
	}
	AnswerBorder->SetBrushColor(FLinearColor(0.964686f, 0.102242f, 0.102242f));
}

void UQuizAnswerUI::SetAnswerResult(bool TrueCorrectOrFalseWrong)
{
	if (TrueCorrectOrFalseWrong)
	{
		JudgingAnswerText->BaseText->SetText(FText::FromString("정답!)"));
		return;
	}
	JudgingAnswerText->BaseText->SetText(FText::FromString("오답!)"));
}

void UQuizAnswerUI::SetQuizAnswer(bool TrueCorrectOrFalseWrong)
{
		if (TrueCorrectOrFalseWrong)
		{
			AnswerText->BaseText->SetText(FText::FromString("정답은 O 입니다.)"));
			return;
		}
	AnswerText->BaseText->SetText(FText::FromString("정답은 X 입니다.)"));
}

void UQuizAnswerUI::SetAnswerCommentary(const FString& NewCommentary)
{
	CommentaryText->SetText(FText::FromString(NewCommentary));
}
