// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/OXQuiz/QuizAnswerUI.h"

#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "UI/BaseUI/BaseText.h"

FLinearColor UQuizAnswerUI::HexToLinearColor(const FString& Hex)
{
	FString CleanHex = Hex.Replace(TEXT("#"), TEXT(""));
	if (CleanHex.Len() != 6 && CleanHex.Len() != 8)
		return FLinearColor::White;

	uint32 HexValue = FParse::HexNumber(*CleanHex);

	if (CleanHex.Len() == 6)
	{
		uint8 R = (HexValue >> 16) & 0xFF;
		uint8 G = (HexValue >> 8) & 0xFF;
		uint8 B = HexValue & 0xFF;

		return FLinearColor(R / 255.f, G / 255.f, B / 255.f, 1.f);
	}
	else // 8자리 -> ARGB
	{
		uint8 A = (HexValue >> 24) & 0xFF;
		uint8 R = (HexValue >> 16) & 0xFF;
		uint8 G = (HexValue >> 8) & 0xFF;
		uint8 B = HexValue & 0xFF;

		return FLinearColor(R / 255.f, G / 255.f, B / 255.f, A / 255.f);
	}
}


void UQuizAnswerUI::SetAnswerColor(bool TrueGreenOrFalseRed)
{
	if (TrueGreenOrFalseRed)
	{
		FLinearColor Color = HexToLinearColor("067C1EFF");
		AnswerBorder->SetBrushColor(Color);
		return;
	}
	FLinearColor Color = HexToLinearColor("F61A1AFF");
	AnswerBorder->SetBrushColor(Color);
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
