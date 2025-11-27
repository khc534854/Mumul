// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatUI.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API UChatUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void OnTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	// 채팅 내용 담고 있는 ScrollBox
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UScrollBox> ScrollBox;
	// 채팅 내용 입력 하는 EditableTextBox
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<class UEditableTextBox> EditBox;
	
	// ChatWidget 블루프린트
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UChatMessageBlockUI> ChatMessageBlockUIClass;
	// 채팅 UI 추가 함수
	void AddChat(FString Text);

};
