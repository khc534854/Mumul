#include "UI/BotChatMessageBlockUI.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"

FString UBotChatMessageBlockUI::ParseInlineStyle(
    const FString& InSource,
    const FString& Delimiter,
    const FString& TagOpen,
    const FString& TagClose
) const
{
    FString Result = InSource;
    int32 SearchIndex = 0;
    bool bOpen = true;

    while (true)
    {
        int32 Found = Result.Find(
            Delimiter,
            ESearchCase::CaseSensitive,
            ESearchDir::FromStart,
            SearchIndex
        );

        if (Found == INDEX_NONE)
            break;

        Result.RemoveAt(Found, Delimiter.Len());

        if (bOpen)
        {
            Result.InsertAt(Found, TagOpen);
            SearchIndex = Found + TagOpen.Len();
        }
        else
        {
            Result.InsertAt(Found, TagClose);
            SearchIndex = Found + TagClose.Len();
        }

        bOpen = !bOpen;
    }

    return Result;
}

void UBotChatMessageBlockUI::SetContent(
    const FString& CurrentTime,
    const FString& Name,
    const FString& Content
) const
{
    if (PlayerName)
        PlayerName->SetText(FText::FromString(Name));

    if (Time)
        Time->SetText(FText::FromString(CurrentTime));

    if (!TextContent)
        return;

    // ===== 원본 문자열 정리 =====
    FString Processed = Content;
    Processed.ReplaceInline(TEXT("\r"), TEXT("")); // Windows 개행 정리

    // 빈 줄 유지한 채 라인 분리
    TArray<FString> Lines;
    Processed.ParseIntoArrayLines(Lines, false);

    FString FinalText;

    for (int32 i = 0; i < Lines.Num(); ++i)
    {
        FString Line = Lines[i];
        bool bIsHeader = false;

        // ===== [1] 블록 스타일 먼저 처리 =====

        if (Line.StartsWith(TEXT("### ")))
        {
            Line.RightChopInline(4);
            Line = FString::Printf(TEXT("<H3>%s</>"), *Line);
            bIsHeader = true;
        }
        else if (Line.StartsWith(TEXT("## ")))
        {
            Line.RightChopInline(3);
            Line = FString::Printf(TEXT("<H2>%s</>"), *Line);
            bIsHeader = true;
        }
        else if (Line.StartsWith(TEXT("# ")))
        {
            Line.RightChopInline(2);
            Line = FString::Printf(TEXT("<H1>%s</>"), *Line);
            bIsHeader = true;
        }
        else if (Line.StartsWith(TEXT("- ")))
        {
            // ❗ RichText 안정화를 위해 '-' 제거
            Line.RightChopInline(2);
            Line = FString::Printf(TEXT("• %s"), *Line);
        }

        // ===== [2] 인라인 스타일 =====
        Line = ParseInlineStyle(Line, TEXT("**"), TEXT("<Bold>"), TEXT("</>"));
        Line = ParseInlineStyle(Line, TEXT("*"),  TEXT("<Italic>"), TEXT("</>"));
        Line = ParseInlineStyle(Line, TEXT("`"),  TEXT("<Code>"), TEXT("</>"));

        // ===== [3] 최종 합치기 =====
        FinalText += Line;

        if (i < Lines.Num() - 1)
        {
            // RichTextBlock은 '\n'이 가장 안전
            FinalText += bIsHeader ? TEXT("\n\n") : TEXT("\n");
        }
    }

    TextContent->SetText(FText::FromString(FinalText));
}



// void UBotChatMessageBlockUI::SetContent(const FString& CurrentTime, const FString& Name, const FString& Content) const
// {
//    TextContent->SetText(FText::FromString(Content));
//    PlayerName->SetText(FText::FromString(Name));
//    Time->SetText(FText::FromString(CurrentTime));
// }
