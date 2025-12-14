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
	CommentaryBorder->SetBrushColor(FLinearColor(0.752942f, 0.056128f, 0.051269f));
}

void UAnswerCommentaryUI::SetQuestion(const int32& QuestionIdx, const FString& NewQuestion)
{
	QuestionText->SetText(FText::FromString(FString::Printf(TEXT("Q%d. %s"), QuestionIdx + 1, *NewQuestion)));
}

void UAnswerCommentaryUI::SetAnswer(const bool& TrueCorrectOrFalseWrong)
{
	if (TrueCorrectOrFalseWrong)
	{
		AnswerText->SetText(FText::FromString(TEXT("정답: O")));
		return;
	}
	AnswerText->SetText(FText::FromString(TEXT("정답: X")));
}

void UAnswerCommentaryUI::SetCommentary(const FString& NewCommentary)
{
	CommentaryText->SetText(FText::FromString(NewCommentary));
}
