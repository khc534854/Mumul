// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/AnswerCommentaryUI.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"

void UAnswerCommentaryUI::SetCommentaryColor(bool TrueGreenOrFalseRed)
{
	if (TrueGreenOrFalseRed)
	{
		CommentaryBorder->SetBrushColor(FLinearColor(0.013702f, 0.015209f, 0.029557f));
		return;
	}
	CommentaryBorder->SetBrushColor(FLinearColor(0.964686f, 0.102242f, 0.102242f));
}

void UAnswerCommentaryUI::SetQuestion(const FString& NewQuestion)
{
	QuestionText->SetText(FText::FromString(NewQuestion));
}

void UAnswerCommentaryUI::SetAnswer(const bool& TrueCorrectOrFalseWrong)
{
	if (TrueCorrectOrFalseWrong)
	{
		AnswerText->SetText(FText::FromString(TEXT("정답: O)")));
		return;
	}
	AnswerText->SetText(FText::FromString(TEXT("정답: X)")));
}

void UAnswerCommentaryUI::SetCommentary(const FString& NewCommentary)
{
	CommentaryText->SetText(FText::FromString(NewCommentary));
}
