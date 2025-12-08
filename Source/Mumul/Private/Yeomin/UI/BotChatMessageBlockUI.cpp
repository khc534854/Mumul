#include "Yeomin/UI/BotChatMessageBlockUI.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"

// FString ParseInlineStyle(const FString& InSource, const FString& Delimiter, const FString& TagOpen, const FString& TagClose)
// {
//     FString Processed = InSource;
//     int32 SearchIndex = 0;
//     bool bIsTagOpen = true;
//
//     while (true)
//     {
//        int32 FoundIdx = Processed.Find(Delimiter, ESearchCase::CaseSensitive, ESearchDir::FromStart, SearchIndex);
//        if (FoundIdx == INDEX_NONE) break;
//
//        Processed.RemoveAt(FoundIdx, Delimiter.Len());
//
//        if (bIsTagOpen)
//        {
//           Processed.InsertAt(FoundIdx, TagOpen);
//           SearchIndex = FoundIdx + TagOpen.Len();
//        }
//        else
//        {
//           Processed.InsertAt(FoundIdx, TagClose);
//           SearchIndex = FoundIdx + TagClose.Len();
//        }
//        bIsTagOpen = !bIsTagOpen;
//     }
//     return Processed;
// }
//
// void UBotChatMessageBlockUI::SetContent(const FString& CurrentTime, const FString& Name, const FString& Content) const
// {
//     if (PlayerName) PlayerName->SetText(FText::FromString(Name));
//     if (Time) Time->SetText(FText::FromString(CurrentTime));
//  
//     if (!TextContent) return;
//  
//     // [핵심 수정 1] 텍스트 처리를 위한 임시 변수
//     FString ProcessedContent = Content;
//  
//     // RichTextBlock이 제대로 인식하도록 \r 문자를 제거합니다.
//     ProcessedContent.ReplaceInline(TEXT("\r"), TEXT(""));
//  
//     // 2. 줄 단위 파싱 (헤더, 리스트 등)
//     TArray<FString> Lines;
//     // ProcessedContent를 기준으로 줄을 나눔
//     ProcessedContent.ParseIntoArray(Lines, TEXT("\n"), false); 
//  
//     FString FinalString;
//     
//     for (int32 i = 0; i < Lines.Num(); ++i)
//     {
//        FString Line = Lines[i];
//        bool bIsHeader = false;
//
//        // --- [A] 줄 단위 스타일 처리 ---
//
//        // 1. 헤더 (###, ##, #)
//        if (Line.StartsWith(TEXT("### ")))
//        {
//           Line.RightChopInline(4); // "### " 제거
//           Line = FString::Printf(TEXT("<H3>%s</>"), *Line);
//           bIsHeader = true;
//        }
//        else if (Line.StartsWith(TEXT("## ")))
//        {
//           Line.RightChopInline(3); // "## " 제거
//           Line = FString::Printf(TEXT("<H2>%s</>"), *Line);
//           bIsHeader = true;
//        }
//        else if (Line.StartsWith(TEXT("# ")))
//        {
//           Line.RightChopInline(2); // "# " 제거
//           Line = FString::Printf(TEXT("<H1>%s</>"), *Line);
//           bIsHeader = true;
//        }
//        // 2. 리스트 (- )
//        else if (Line.StartsWith(TEXT("- ")))
//        {
//           Line.RightChopInline(2); // "- " 제거
//           // 리스트 아이콘이나 들여쓰기 스타일 적용 (예: <List>• 내용</>)
//           Line = FString::Printf(TEXT("  • %s"), *Line); 
//        }
//
//        // --- [B] 문장 내 스타일 처리 (순서 중요: ** 먼저, 그 다음 *) ---
//        
//        // 3. Bold (**)
//        Line = ParseInlineStyle(Line, TEXT("**"), TEXT("<Bold>"), TEXT("</>"));
//
//        // 4. Italic (*) -> Bold 처리 후 남은 * 처리
//        Line = ParseInlineStyle(Line, TEXT("*"), TEXT("<Italic>"), TEXT("</>"));
//        
//        // 5. Inline Code (`) 
//        Line = ParseInlineStyle(Line, TEXT("`"), TEXT("<Code>"), TEXT("</>"));
//
//        // --- [C] 최종 합치기 ---
//        FinalString += Line;
//
//        // 마지막 줄이 아니라면 줄바꿈 추가
//        // 헤더 뒤에는 줄바꿈을 두 번 넣어 간격을 줄 수도 있음
//        if (i < Lines.Num() - 1)
//        {
//           FinalString += (bIsHeader) ? TEXT("<br/><br/>") : TEXT("<br/>");
//        }
//     }
//
//     // 최종 적용
//     TextContent->SetText(FText::FromString(FinalString));
// }

void UBotChatMessageBlockUI::SetContent(const FString& CurrentTime, const FString& Name, const FString& Content) const
{
   TextContent->SetText(FText::FromString(Content));
   PlayerName->SetText(FText::FromString(Name));
   Time->SetText(FText::FromString(CurrentTime));
}
