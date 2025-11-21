// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateSessionWidget.generated.h"

class UEditableTextBox;
class UButton;


UCLASS()
class MUMUL_API UCreateSessionWidget : public UUserWidget
{
	GENERATED_BODY()
// protected:
// 	virtual void NativeConstruct() override;

// private:
// 	UFUNCTION()
// 	void OnClickCreateButton();
//
// protected:
// 	// 위젯 바인딩 요소
// 	UPROPERTY(meta = (BindWidget))
// 	TObjectPtr<UEditableTextBox> SessionNameInput; // 세션 이름 입력 (DisplayName)
//
// 	UPROPERTY(meta = (BindWidget))
// 	TObjectPtr<UEditableTextBox> MaxPlayersInput; // 최대 플레이어 수 입력
//
// 	UPROPERTY(meta = (BindWidget))
// 	TObjectPtr<UButton> CreateSessionButton; // 세션 생성 요청 버튼
//
// private:
// 	// 디폴트 맵 정보 (예시: 모든 맵 정보를 담고 있는 Data Asset에서 로비 맵 선택)
// 	// 실제 구현에서는 MapDataAsset을 참조하여 원하는 게임 맵을 선택해야 합니다.
};
